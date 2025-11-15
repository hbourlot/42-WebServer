#include "httpTcpServer/HttpTcpServerLinux.hpp"

const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations ) {

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for ( size_t i = 0; i < locations.size(); ++i ) {

		const std::string &locPath = locations[ i ].path;

		if ( path.compare( 0, locPath.size(), locPath ) == 0 && locPath.size() > matchLength ) {
			matchedLocation = &locations[ i ];
			matchLength = locPath.size();
		}
	}
	return ( matchedLocation );
}

static bool validateRequestMethod( const httpRequest &request, const Location &location ) {

	if ( request.method != "GET" && request.method != "POST" && request.method != "DELETE" )
		return false;

	for ( size_t i = 0; i < location.methods.size(); ++i ) {
		if ( request.method == location.methods[ i ] )
			return true;
	}
	return false;
}

VALIDATION_STATUS http::Router::validateRequest( Client &client, const ServerConfig &server ) {

	httpRequest &request = client.getRequest();
	http::Response &response = client.getResponse();

	request.urlMatchedLocation = getMatchLocation( request.path, server.locations );

	if ( !request.urlMatchedLocation ) // URL NOT FOUND
		return VALID_NOT_FOUND;

	const Location &matchedLocation = *request.urlMatchedLocation;

	if ( !request.urlMatchedLocation->redirection.empty() ) // /redirect-me
		return VALID_REDIRECT_REQUIRED;

	// !!!!CGI
	if ( !request.urlMatchedLocation->cgi_extension.empty() )
		return VALID_IS_CGI;

	if ( !validateRequestMethod( request, matchedLocation ) )
		return VALID_METHOD_NOT_ALLOWED;

	return VALID_OK;
}

void http::Router::routeRequest( Client &client, const ServerConfig &server, ClientEventProcessor &processor ) {

	httpRequest &request = client.getRequest();
	const Location &location = *( client.getRequest().urlMatchedLocation );

	if ( isCgiRequest( request, location ) )
		return handleCgiRequest( client, server, location, processor );

	return handleStaticRequest( client, server, location ); // Static (GET, POST, DELETE)
}

bool http::Router::isCgiRequest( const httpRequest &request, const Location &location ) {

	if ( location.cgi_extension.empty() ) { // Checking if location has CGI configured
		return false;
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

void http::Router::handleCgiRequest( Client &client, const ServerConfig &server, const Location &location,
                                     ClientEventProcessor &processor ) {

	httpRequest &request = client.getRequest();

	if ( ( request.method == "GET" || request.method == "POST" ) && client.getState() != CGI_IN_EXECUTION ) {
		client.setState( CGI_IN_EXECUTION );

		httpRequest &request = client.getRequest();
		std::string path = location.path;
		sockaddr_in socket;
		Cgi myCgi( request, path, socket, server );

		myCgi.executeCgi( client.getServer()._fds );

		// Store CGI info in client
		client.setCgiPid( myCgi.getPid() );
		client.setCgiOutputFd( myCgi.getOutputPipe()[ 0 ] );

		// Register CGI pipes for proper cleanup
		processor.registerCgiPipes( myCgi.getInputPipe(), myCgi.getOutputPipe(), myCgi.getPid() );

		// Map CGI fd to client for event loop lookup
		processor.registerCgiForClient( client, client.getCgiOutputFd() );

		// myCgi.readCgiOutput();
		// std::cout << "\n\nmyCgi Response -------------\n\n";
		// std::cout << myCgi.getResponse().buildResponseString() << std::endl;
		// std::cout << "\n\nmyCgi Response END ----------\n\n";

		// if ( myCgi.processCgiOut() ) {
		// 	std::cout << "Inside 'if scoop' in ProcessCgiOut()\n";
		// 	client.getResponse() = myCgi.getResponse();
		// }
	} else {
		std::cout << "Else case in handleCgiRequest()\n";
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
}

void http::Router::handleStaticRequest( Client &client, const ServerConfig &server, const Location &location ) {

	httpRequest &request = client.getRequest();

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

	httpRequest &request = client.getRequest();
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
	http::Response &response = client.getResponse();
	httpRequest &request = client.getRequest();
	std::string ContentType;

	if ( location.uploadEnable ) {
		UploadManager::handleUpload( location, client, serverInfo );
	} else if ( !location.uploadEnable ) {
		client.getResponse().buildErrorResponse( HTTP_UPLOAD_FORBID, serverInfo );
	} else {
		client.getResponse().buildErrorResponse( HTTP_NOT_FOUND, serverInfo );
	}
}

void http::Router::handleDelete( Client &client, const ServerConfig &server, const Location &location ) {
	http::Response &response = client.getResponse();
	httpRequest &request = client.getRequest();

	std::string filePath = getFilePath( request.path, location );

	std::cout << filePath << std::endl;
	if ( isDirectory( filePath ) ) {
		std::cout << "Is a dir cannot delete" << std::endl;
		// return (false);
	}
	if ( remove( filePath.c_str() ) )
		std::cout << "Files not delete" << std::endl;
	// return (true);
}

// ! Did i make this?
std::string parseContentType( std::string &contentType ) {
	std::string parsedContentType;

	parsedContentType = contentType.substr( 0, contentType.find( ';' ) );

	return ( parsedContentType );
}


