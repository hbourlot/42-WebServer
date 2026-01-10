#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

// Used on 'getMatchLocation to check if we choose the right path
// For the cases with multiple cgi-bin paths for example
std::string GetExtension( const std::string &path ) {
	std::string::size_type pos = path.rfind( '.' ); // Finds the last '.'
	if ( pos == std::string::npos )
		return ""; // No extension on path

	return path.substr( pos + 1 );
}

MatchResult getMatchResult( const std::string &path, const ServerConfig &server ) {
	MatchResult result;
	result.file = NULL;
	result.location = NULL;

	result.file = getMatchFile( path, server.files );
	if ( result.file != NULL )
	return result;
	std::cout << "Confia: "<< path << std::endl;
	result.location = getMatchLocation( path, server.locations );
	return result;
}

static bool validateRequestMethod( const http::Request &request, const Location &location ) {

	if ( request.method != "GET" && request.method != "POST" && request.method != "DELETE" )
		return false;

	for ( size_t i = 0; i < location.methods.size(); ++i ) {
		if ( request.method == location.methods[ i ] )
			return true;
	}
	return false;
}
static bool validateRequestMethod( const http::Request &request, const File &file ) {
	std::cout<< "Er" <<std::endl;
	if ( request.method != "GET" && request.method != "POST" && request.method != "DELETE" )
		return false;

	for ( size_t i = 0; i < file.methods.size(); ++i ) {
		if ( request.method == file.methods[ i ] )
			return true;
	}
	return false;
}

VALIDATION_STATUS http::Router::validateRequest( Client &client, const ServerConfig &server ) {

	http::Request &request = client.getRequest();

	request.matchResult = getMatchResult( request.path, server );

	if ( !request.matchResult.file && !request.matchResult.location )
		return VALID_NOT_FOUND;

	if ( request.matchResult.file != NULL ) {
		if ( !validateRequestMethod( request, *request.matchResult.file ) )
			return VALID_METHOD_NOT_ALLOWED;

		return VALID_OK;
	}
	    const Location &loc = *request.matchResult.location;

    if (!loc.redirection.empty())
        return VALID_REDIRECT_REQUIRED;

    if (!loc.cgi_extension.empty())
        return VALID_IS_CGI;

    if (!validateRequestMethod(request, loc))
        return VALID_METHOD_NOT_ALLOWED;

    return VALID_OK;

}

bool http::Router::routeCgiRequest( Client &client, const ServerConfig &server, const Location &location,
                                    ClientEventProcessor &processor ) {

	http::Request &request = client.getRequest();

	if ( request.method == "GET" || request.method == "POST" ) {
		launchCgi( client, server, location, processor );
		return false;

	} else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
	return true;
}

bool http::Router::routeCgiRequest( Client &client, const ServerConfig &server, const File &file,
                                    ClientEventProcessor &processor ) {

	http::Request &request = client.getRequest();

	if ( request.method == "GET" || request.method == "POST" ) {
		launchCgi( client, server, file, processor );
		return false;

	} else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
	return true;
}

void http::Router::launchCgi( Client &client, const ServerConfig &server, const Location &location,
                              ClientEventProcessor &processor ) {
	http::Request &request = client.getRequest();
	std::string path = location.path;

	// Create and execute CGI
	http::Cgi *cgi = new http::Cgi( request, path, server, &client );
	cgi->executeCgi( client.getServer()._fds );

	// Store CGI info in client
	client.setCgiPid( cgi->getPid() );
	client.setCgiOutputFd( cgi->getOutputPipe()[ 0 ] );

	// Register CGI in map (takes ownership)
	processor.registerCgi( cgi );
	client.setState( CGI_JUST_STARTED );
}

void http::Router::launchCgi( Client &client, const ServerConfig &server, const File &file,
                              ClientEventProcessor &processor ) {
	http::Request &request = client.getRequest();
	std::string path = file.extension; // /cgi-bin --> *.bla

	// Create and execute CGI
	http::Cgi *cgi = new http::Cgi( request, path, server, &client );
	cgi->executeCgi( client.getServer()._fds );

	// Store CGI info in client
	client.setCgiPid( cgi->getPid() );
	client.setCgiOutputFd( cgi->getOutputPipe()[ 0 ] );

	// Register CGI in map (takes ownership)
	processor.registerCgi( cgi );
	client.setState( CGI_JUST_STARTED );
}

void http::Router::routeStaticRequest( Client &client, const ServerConfig &server, const Location &location ) {

	http::Request &request = client.getRequest();

	if ( request.method == "GET" ) {
		return ( handleGet( client, server, location ) );
	} else if ( request.method == "POST" )
		return ( handlePost( client, server, location ) );
	else if ( request.method == "DELETE" )
		return ( handleDelete( client, server, location ) );
	else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
}

void http::Router::handleGet( Client &client, const ServerConfig &server, const Location &location ) {

	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	std::string filePath = getFilePath( request.path, location );

	if ( isDirectory( filePath ) ) {
		handleDirectoryListing( client, server, filePath, location );
		return;
	}

	if ( !std::ifstream( filePath.c_str() ).is_open() ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		return;
	}

	response.buildFileResponse( HTTP_OK, filePath, server );
}

void http::Router::handlePost( Client &client, const ServerConfig &serverInfo, const Location &location ) {
	std::string ContentType;

	if ( location.uploadEnable ) {
		UploadManager::handleUpload( location, client, serverInfo );
	} else if ( !location.uploadEnable ) {
		client.getResponse().buildErrorResponse( HTTP_FORBID, serverInfo );
	} else {
		client.getResponse().buildErrorResponse( HTTP_NOT_FOUND, serverInfo );
	}
}

void http::Router::handleDelete( Client &client, const ServerConfig &server, const Location &location ) {
	http::Response &response = client.getResponse();
	http::Request &request = client.getRequest();

	std::string filePath = getFilePath( request.path, location );

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
