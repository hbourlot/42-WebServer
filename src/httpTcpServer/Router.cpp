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

static bool isCgirequest( const http::Request &request, const Location &location ) {

	for ( size_t i = 0; i < location.cgi_extension.size(); ++i )
		if ( location.cgi_extension[ i ] == ".*" ) { // ".cgi" accept any kind of cgi
			return true;
		}

	// Extract file extension from the request path
	std::string path = request.path;
	size_t dotPos = path.find_last_of( '.' );

	if ( dotPos == std::string::npos ) {
		return false; // No extension found
	}

	std::string extension = path.substr( dotPos ); // Includes the dot (.py, .cgi, etc.)

	// Check if the extension is in the location's CGI extensions
	for ( size_t i = 0; i < location.cgi_extension.size(); ++i ) {
		if ( location.cgi_extension[ i ] == extension ) {
			return true;
		}
	}

	return false;
}

VALIDATION_STATUS http::Router::validateRequest( Client &client, const ServerConfig &server ) {

	http::Request &request = client.getRequest();

	request.matchResult.location = getMatchLocation( request.path, server.locations );
	request.matchResult.file = getMatchFile( request.path, server.files );
	if ( request.matchResult.file ) {
		if ( !request.matchResult.file->cgi_pass.empty() &&
		     validateRequestMethod( request, request.matchResult.file->methods ) )
			return VALID_IS_CGI;
	}

	if ( request.matchResult.location ) {

		if ( !request.matchResult.location->redirection.empty() )
			return VALID_REDIRECT_REQUIRED;

		std::vector< std::string > methods = resolveMethods( request.matchResult.location, request.matchResult.file );

		if ( !validateRequestMethod( request, methods ) ) {
			return VALID_METHOD_NOT_ALLOWED;
		}

		if ( !request.matchResult.location->cgi_extension.empty() ) {
			if ( isCgirequest( request, *request.matchResult.location ) )
				return VALID_IS_CGI;
			else
				return VALID_FORBIDDEN;
		}

		return VALID_OK;
	}
	return VALID_OK;
}

bool http::Router::routeCgiRequest( Client &client, const ServerConfig &server, const RouteContext &ctx,
                                    ClientEventProcessor &processor ) {

	http::Request &request = client.getRequest();
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
	http::Cgi *cgi = new http::Cgi( request, ctx.cgi_pass, server, &client );
	cgi->executeCgi();

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
		std::cout << "ctx.uploadEnable\n";
		UploadManager::handleUpload( ctx, client, serverInfo );
	} else if ( !ctx.uploadEnable ) {
		std::cout << "!ctx.uploadEnable\n";
		client.getResponse().buildErrorResponse( HTTP_FORBID, serverInfo );
	} else {
		std::cout << "else case\n";
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
