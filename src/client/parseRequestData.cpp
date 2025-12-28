#include "Client/ClientEventProcessor.hpp"

static void parseRequestQueries( http::Request& request ) {
	std::string fullPath = request.path;
	std::string::size_type qpos = fullPath.find( '?' );

	if ( qpos != std::string::npos ) {
		request.queryString = fullPath.substr( qpos + 1 );
		request.path = fullPath.substr( 0, qpos );
	} else {
		request.queryString = "";
	}
}

static std::string getPathInfo( std::string& requestPath, std::string scriptExt ) {

	std::string pathInfo;
	std::string::size_type extPos = requestPath.find( scriptExt );
	std::cout << "scriptExt : " << scriptExt << std::endl;
	if ( extPos != std::string::npos ) {
		extPos += scriptExt.length();

		if ( extPos < requestPath.length() ) {
			pathInfo = requestPath.substr( extPos );
		}
	}

	return pathInfo.empty() ? "" : pathInfo;
}

static std::string getPathTranslated( const std::string& root, const std::string& pathInfo ) {

	std::string result = root;
	std::string::size_type idx = result.length() - 1;
	if ( !result.empty() && result[ idx ] == '/' && !pathInfo.empty() && pathInfo[ 0 ] == '/' )
		result[ idx ] = '\0';
	return result + pathInfo;
}

static void parsePath( http::Request& request, const ServerConfig& serverInfo ) {
	std::string& requestPath = request.path;
	std::string pathInfo = "";

	for ( size_t i = 0; i < serverInfo.locations.size(); ++i ) {
		if ( !serverInfo.locations[ i ].cgi_extension.empty() ) {
			std::vector< std::string > scriptExt = serverInfo.locations[ i ].cgi_extension;
			for ( size_t j = 0; j < scriptExt.size(); ++j ) {
				std::string& value = scriptExt[ j ];
				std::string::size_type extPos = requestPath.find( value );

				if ( requestPath.find( value ) != std::string::npos &&
				     requestPath.find( value ) + value.length() <= requestPath.length() ) {

					char nextChar = requestPath[ extPos + value.length() ];
					if ( nextChar != '/' ) {
						continue;
					}

					request.pathInfo = getPathInfo( requestPath, value );
					std::cout << "parsePath => request.pathInfo => " << request.pathInfo << std::endl;
					if ( !request.pathInfo.empty() ) {
						requestPath.erase( requestPath.length() - request.pathInfo.length() );
						request.pathTranslated = getPathTranslated( serverInfo.locations[ i ].root, request.pathInfo );
					}
					return;
				}
			}
		}
	}
	if ( request.pathInfo.empty() ) {
		request.pathInfo = "";
		request.pathTranslated = "";
	}
}

static void parseRequestHeaders( http::Request& request, std::istringstream& request_stream, std::string& line ) {
	size_t idx;

	while ( std::getline( request_stream, line ) ) {
		if ( line == "\r" || line == "" )
			break;
		idx = line.find( ":" );
		if ( idx != std::string::npos ) {

			std::string key = line.substr( 0, idx );
			std::string value = line.substr( idx + 1 );
			key = ft_strtrim( key );
			value = ft_strtrim( value );
			request.headers[ key ] = value;
		}
	}
}

std::string http::Request::GetFileName() {
	std::string::size_type pos = path.rfind( '/' );
	if ( pos == std::string::npos )
		return path;
	return path.substr( pos + 1 );
}

bool http::ClientEventProcessor::parseRequestData( Client& client, const ServerConfig& serverInfo ) {

	http::Request& clientRequest = client.getRequest();
	std::istringstream requestStream( client.getReadBuffer() );
	std::string line;

	if ( !std::getline( requestStream, line ) ) {
		client.setState( PARSE_INCOMPLETE );
		return false;
	}

	std::istringstream firstLine( line );

	firstLine >> clientRequest.method >> clientRequest.path >> clientRequest.serverProtocol;

	parseRequestQueries( clientRequest );
	parsePath( clientRequest, serverInfo );
	parseRequestHeaders( clientRequest, requestStream, line );

	std::string body;
	while ( std::getline( requestStream, line ) )
		body += line + "\n";
	clientRequest.body = body;

	if ( clientRequest.headers.count( "Content-Length" ) ) {
		size_t contentLength = std::strtoul( clientRequest.headers[ "Content-Length" ].c_str(), NULL, 10 );
		if ( contentLength > serverInfo.maxRequest * 1024 * 1024 ) {
			client.setState( PARSE_TOO_LARGE );
			return true;
		}
		if ( clientRequest.body.size() < contentLength ) {
			client.setState( PARSE_INCOMPLETE );
			return false;
		}
	}

	client.getResponse() = Response( clientRequest );
	ensureSessionId( client );

	client.setState( PARSE_OK );

	return true;
}