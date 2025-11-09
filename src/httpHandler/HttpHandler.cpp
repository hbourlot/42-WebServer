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

VALIDATION_STATUS HttpRouter::validateRequest( Client &client, const ServerConfig &server ) {

	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	request.urlMatchedLocation = getMatchLocation( request.path, server.locations );

	if ( !request.urlMatchedLocation ) // URL NOT FOUND
		return VALID_NOT_FOUND;

	const Location &matchedLocation = *request.urlMatchedLocation;

	if ( !request.urlMatchedLocation->redirection.empty() ) // /redirect-me
		return VALID_REDIRECT_REQUIRED;

		
	// !!!!CGI
	if (!request.urlMatchedLocation->cgi_extension.empty()){
		std::cerr << "Encontrei CGI valido" << std::endl;
		return VALID_IS_CGI;
	} // Improve Miguel

	if ( !validateRequestMethod( request, matchedLocation ) )
		return VALID_METHOD_NOT_ALLOWED;

	return VALID_OK;
}

void HttpRouter::routeRequest( Client &client, const ServerConfig &server ) {

	httpRequest &request = client.getRequest();
	const Location &location = *( client.getRequest().urlMatchedLocation );
	std::cerr << "vou validar CGI" << std::endl;
	if ( isCgiRequest( request, location ) )
		return handleCgiRequest( client, server, location );

	return handleStaticRequest( client, server, location );
}

bool HttpRouter::isCgiRequest( const httpRequest &request, const Location &location ) {
	// Check file extension (.php, .py, .cgi, etc.)
	std::string filePath = getFilePath( request.path, location );
	// return isValidCgiExtension( filePath );
	return false;
}

void HttpRouter::handleCgiRequest( Client &client, const ServerConfig &server, const Location &location ) {

	httpRequest &request = client.getRequest();
	std::string path = location.path;
	if ( request.method == "GET" || request.method == "POST" ) {
		// !!CGI
		// sockaddr_in socket;
		// http::Cgi cgi(request, path, socket, server); // Working on
		// cgi.executeCgi(); // Working on (what to pass as a parameter)
		// client.setState(CGI_IN_EXECUTION);
	} else {
		client.getResponse() = ResponseBuilder::buildErrorResponse( HTTP_FORBID_METHOD );
	}
}

void HttpRouter::handleStaticRequest( Client &client, const ServerConfig &server, const Location &location ) {

	httpRequest &request = client.getRequest();

	if ( request.method == "GET" ) {
		return ( handleGet( client, server, location ) );
	} else if ( request.method == "POST" )
		return ( handlePost( client, server, location ) );
	else if ( request.method == "DELETE" )
		return ( handleDelete( client, server, location ) );
	else {
		client.getResponse() = ResponseBuilder::buildErrorResponse( HTTP_FORBID_METHOD );
	}
}

void HttpRouter::handleGet( Client &client, const ServerConfig &server, const Location &location ) {

	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	std::string filePath = getFilePath( request.path, location );

	if ( isDirectory( filePath ) ) {
		handleDirectoryListing( client, server, filePath, location );
		return;
	}

	if ( !std::ifstream( filePath.c_str() ).is_open() ) {
		response = ResponseBuilder::buildFileResponse( HTTP_NOT_FOUND, server.errorPage.at( 404 ), server, true );
		return;
	}

	response = ResponseBuilder::buildFileResponse( HTTP_OK, filePath, server );
}

void HttpRouter::handlePost( Client &client, const ServerConfig &serverInfo, const Location &location ) {
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();
	std::string ContentType;

	if ( location.uploadEnable ) {
		UploadManager::handleUpload( location, client, serverInfo );
	} else if ( !location.uploadEnable ) {
		client.getResponse() = ResponseBuilder::buildErrorResponse( HTTP_UPLOAD_FORBID );
	} else {
		client.getResponse() =
		    ResponseBuilder::buildFileResponse( HTTP_NOT_FOUND, serverInfo.errorPage.at( 404 ), serverInfo, true );
	}
}

void HttpRouter::handleDelete( Client &client, const ServerConfig &server, const Location &location ) {
	httpResponse &response = client.getResponse();
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
