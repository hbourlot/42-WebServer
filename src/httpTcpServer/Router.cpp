#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

// Used on 'getMatchLocation to check if we choose the right path
// For the cases with multiple cgi-bin paths for example
// std::string GetExtension( const std::string &path ) {
// 	std::string::size_type pos = path.rfind( '.' ); // Finds the last '.'
// 	if ( pos == std::string::npos )
// 		return ""; // No extension on path

// 	return path.substr( pos + 1 );
// } // ! Have One overload on utils

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

	if ( request.matchResult.file ) {
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
	cgi->executeCgi( client.getServer()._fds );

	// Send Body if has
	std::cerr << "\n\n BEFORE WRITE INTO CGI\n"
	          << "Body size " << request.body.size() << std::endl;

	write( cgi->getInputPipe()[ 1 ], request.body.c_str(), request.body.size() );
	std::cout << "\n\n after/2 WRITE INTO CGI\n";

	std::cout << "\n\n AFTER WRITE INTO CGI\n";
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
