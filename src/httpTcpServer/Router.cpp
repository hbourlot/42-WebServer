#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

// Used on 'getMatchLocation to check if we choose the right path
// For the cases with multiple cgi-bin paths for example
std::string GetExtension( const std::string& path ) {
	std::string::size_type pos = path.rfind( '.' ); // Finds the last '.'
	if ( pos == std::string::npos )
		return ""; // No extension on path

	return path.substr( pos + 1 );
}

const Location* getMatchLocation( const std::string& path, const std::vector< Location >& locations ) {

	const Location* matchedLocation = NULL;
	size_t matchLength = 0;

	for ( size_t i = 0; i < locations.size(); ++i ) {

		const std::string& locPath = locations[ i ].path;

		if ( path.compare( 0, locPath.size(), locPath ) == 0 && locPath.size() > matchLength ) {
			matchedLocation = &locations[ i ];
			matchLength = locPath.size();
		}

		// Checks for the correct CGI location, if we don't find the right location, we return the last one found it
		if ( matchedLocation != NULL && matchedLocation->cgi.empty() == false ) {
			// Loops into the vector until we find the Extension cgi, for example ".py"
			std::vector< std::string > cgiExtensions = matchedLocation->cgi_extension;
			if ( std::find( cgiExtensions.begin(), cgiExtensions.end(), GetExtension( path ) ) !=
			     cgiExtensions.end() ) {
				return matchedLocation;
			}
		}
	}
	return ( matchedLocation );
}

static bool validateRequestMethod( const http::Request& request, const Location& location ) {

	if ( request.method != "GET" && request.method != "POST" && request.method != "DELETE" )
		return false;

	for ( size_t i = 0; i < location.methods.size(); ++i ) {
		if ( request.method == location.methods[ i ] )
			return true;
	}
	return false;
}

VALIDATION_STATUS http::Router::validateRequest( Client& client, const ServerConfig& server ) {

	http::Request& request = client.getRequest();

	request.urlMatchedLocation = getMatchLocation( request.path, server.locations );

	if ( !request.urlMatchedLocation ) // URL NOT FOUND
		return VALID_NOT_FOUND;

	const Location& matchedLocation = *request.urlMatchedLocation;

	if ( !request.urlMatchedLocation->redirection.empty() ) // /redirect-me
		return VALID_REDIRECT_REQUIRED;

	if ( !request.urlMatchedLocation->cgi_extension.empty() )
		return VALID_IS_CGI;

	if ( !validateRequestMethod( request, matchedLocation ) )
		return VALID_METHOD_NOT_ALLOWED;

	return VALID_OK;
}

bool http::Router::routeCgiRequest( Client& client, const ServerConfig& server, const Location& location,
                                    ClientEventProcessor& processor ) {

	http::Request& request = client.getRequest();

	if ( request.method == "GET" || request.method == "POST" ) {
		launchCgi( client, server, location, processor );
		return false;

	} else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
	return true;
}

void http::Router::launchCgi( Client& client, const ServerConfig& server, const Location& location,
                              ClientEventProcessor& processor ) {
	http::Request& request = client.getRequest();
	std::string path = location.path;
	sockaddr_in socket;

	if ( location.path.compare( 0, 1, "/" ) == 0 )
		path = location.path.substr( 1 );

	// Create and execute CGI
	http::Cgi* cgi = new http::Cgi( request, path, socket, server, &client );
	cgi->executeCgi( client.getServer()._fds );

	// Store CGI info in client
	client.setCgiPid( cgi->getPid() );
	client.setCgiOutputFd( cgi->getOutputPipe()[ 0 ] );

	// Register CGI in map (takes ownership)
	processor.registerCgi( cgi );
	client.setState( CGI_JUST_STARTED );
}

void http::Router::routeStaticRequest( Client& client, const ServerConfig& server, const Location& location ) {

	http::Request& request = client.getRequest();

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

void http::Router::handleGet( Client& client, const ServerConfig& server, const Location& location ) {

	http::Request& request = client.getRequest();
	http::Response& response = client.getResponse();

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

void http::Router::handlePost( Client& client, const ServerConfig& serverInfo, const Location& location ) {
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
