#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

static bool validateRequestMethod( const http::Request &request, const std::vector< std::string > &methods ) {
	if ( request.method != "GET" && request.method != "POST" && request.method != "DELETE" )
		return false;

	for ( size_t i = 0; i < methods.size(); ++i ) {
		if ( request.method == methods[ i ] )
			return true;
	}
	return false;
}
static std::vector< std::string > resolveMethods( const Location *loc, const File *file ) {
	if ( loc && !loc->methods.empty() )
		return loc->methods;
	if ( file )
		return file->methods;
	return std::vector< std::string >();
}

VALIDATION_STATUS http::Router::validateRequest( Client &client, const ServerConfig &server ) {

	http::Request &request = client.getRequest();

	request.matchResult.location = getMatchLocation( request.path, server.locations );
	request.matchResult.file = getMatchFile( request.path, server.files );
	std::cout << "request.path:" << request.path << std::endl;
	if ( request.matchResult.file ) {
		std::cout << "request.matchResult.file exist" << std::endl;
		if ( !request.matchResult.file->cgi_pass.empty() &&
		     validateRequestMethod( request, request.matchResult.file->methods ) )
			return VALID_IS_CGI;
	}

	if ( request.matchResult.location ) {

		if ( !request.matchResult.location->redirection.empty() )
			return VALID_REDIRECT_REQUIRED;

		std::vector< std::string > methods = resolveMethods( request.matchResult.location, request.matchResult.file );

		if ( !validateRequestMethod( request, methods ) )
			return VALID_METHOD_NOT_ALLOWED;

		if ( !request.matchResult.location->cgi_extension.empty() )
			return VALID_IS_CGI;

		return VALID_OK;
	}
	return VALID_OK;
}

bool http::Router::routeCgiRequest( Client &client, const ServerConfig &server, const RouteContext &ctx,
                                    ClientEventProcessor &processor ) {

	http::Request &request = client.getRequest();
	std::cout << "routeCgiRequest" << std::endl;
	if ( request.method == "GET" || request.method == "POST" ) {
		launchCgi( client, server, ctx, processor );
		return false;
	} else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
	return true;
}

void http::Router::launchCgi( Client &client, const ServerConfig &server, const RouteContext &ctx,
                              ClientEventProcessor &processor ) {
	http::Request &request = client.getRequest();

	// Create and execute CGI
	std::cout << "launchCgi" << std::endl;
	http::Cgi *cgi = new http::Cgi( request, ctx.cgi_pass, server, &client );
	cgi->executeCgi( client.getServer()._fds );

	    size_t len = request.body.size();
    const char* buf = request.body.data();
    int fd = cgi->getInputPipe()[1]; // parent writes to this


size_t total = 0;
	while (total < len) {
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLOUT;
		int poll_res = poll(&pfd, 1, 1000); // 1 second timeout
		if (poll_res < 0) {
			perror("poll");
			break;
		} else if (poll_res == 0) {
			std::cerr << "Timeout waiting for CGI pipe to be writable.\n";
			if (processor.hasCgiFinished(cgi))
				std::cout << "LAELE\n";
			break;
		}
		if (pfd.revents & POLLERR) {
			std::cerr << "Error on CGI pipe fd during poll.\n";
			break;
		}
		if (pfd.revents & POLLHUP) {
			std::cerr << "CGI pipe closed (POLLHUP).\n";
			break;
		}
		if (pfd.revents & POLLOUT) {
			std::cout << "IS POLL OUT AND GOING TO WRITE NOW\n\n";
			size_t to_write = (len - total > CHUNK_SIZE) ? CHUNK_SIZE : (len - total);
			ssize_t n = write(fd, buf + total, to_write);
			std::cout << "DID WRITE A CHUNK\n\n";
			if (n < 0) {
				if (errno == EINTR) {
					std::cout << "OVER HERE\n";
					continue;
				}
				if (errno == EPIPE) {
					std::cerr << "CGI closed stdin, stopping write.\n";
					break;
				}
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					continue;
				}
				perror("write");
				break;
			}
			total += n;
		}
	}

	close(fd);

    std::cout << "\n\nAFTER WRITE INTO CGI\n";
	// Store CGI info in client
	client.setCgiPid( cgi->getPid() );
	client.setCgiOutputFd( cgi->getOutputPipe()[ 0 ] );

	// Register CGI in map (takes ownership)
	processor.registerCgi( cgi );
	client.setState( CGI_JUST_STARTED );
}

void http::Router::routeStaticRequest( Client &client, const ServerConfig &server, const RouteContext &ctx ) {

	http::Request &request = client.getRequest();

	if ( request.method == "GET" )
		return ( handleGet( client, server, ctx ) );
	else if ( request.method == "POST" )
		return ( handlePost( client, server, ctx ) );
	else if ( request.method == "DELETE" )
		return ( handleDelete( client, server, ctx ) );
	else
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
}

void http::Router::handleGet( Client &client, const ServerConfig &server, const RouteContext &ctx ) {

	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	std::string filePath = getFilePath( request.path, ctx );

	if ( isDirectory( filePath ) ) {
		handleDirectoryListing( client, server, filePath, ctx );
		return;
	}

	if ( !std::ifstream( filePath.c_str() ).is_open() ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		return;
	}

	response.buildFileResponse( HTTP_OK, filePath, server );
}

void http::Router::handlePost( Client &client, const ServerConfig &serverInfo, const RouteContext &ctx ) {
	std::string ContentType;

	if ( ctx.uploadEnable ) {
		UploadManager::handleUpload( ctx, client, serverInfo );
	} else if ( !ctx.uploadEnable ) {
		client.getResponse().buildErrorResponse( HTTP_FORBID, serverInfo );
	} else {
		client.getResponse().buildErrorResponse( HTTP_NOT_FOUND, serverInfo );
	}
}

void http::Router::handleDelete( Client &client, const ServerConfig &server, const RouteContext &ctx ) {
	http::Response &response = client.getResponse();
	http::Request &request = client.getRequest();

	std::string filePath = getFilePath( request.path, ctx );

	struct stat st;

	if ( stat( filePath.c_str(), &st ) != 0 ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		Logs::log( LOGS_ERROR, "File Not Found" );
		return;
	}

	if ( isDirectory( filePath ) ) {
		response.buildErrorResponse( HTTP_FORBID, server );
		Logs::log( LOGS_ERROR, "Cannot delete because its a folder" );
		return;
	}
	if ( remove( filePath.c_str() ) ) {
		response.buildErrorResponse( HTTP_SERVER_ERR, server );
		Logs::log( LOGS_ERROR, "Failed to delete File: " + filePath );
		return;
	}
	response.buildResponse( HTTP_NO_CONTENT, "" );
	Logs::log( LOGS_ERROR, "File deleted Successfully " + filePath );
}
