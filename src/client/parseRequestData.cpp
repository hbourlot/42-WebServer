#include "Client/ClientEventProcessor.hpp"

static void parseRequestQueries( http::Request &request ) {
	std::string fullPath = request.path;
	std::string::size_type qpos = fullPath.find( '?' );

	if ( qpos != std::string::npos ) {
		request.queryString = fullPath.substr( qpos + 1 );
		request.path = fullPath.substr( 0, qpos );
	} else {
		request.queryString = "";
	}
}

static void parsePath( http::Request &request, const ServerConfig &serverInfo ) {
	std::string &path = request.path;

	// by default
	request.pathInfo = "";
	request.pathTranslated = "";

	for ( size_t i = 0; i < serverInfo.locations.size(); ++i ) {
		const Location &loc = serverInfo.locations[ i ];
		if ( loc.cgi_extension.empty() )
			continue;

		for ( size_t j = 0; j < loc.cgi_extension.size(); ++j ) {
			const std::string &ext = loc.cgi_extension[ j ];

			size_t pos = path.find( ext );
			if ( pos == std::string::npos )
				continue;

			if ( pos + ext.size() < path.size() && path[ pos + ext.size() ] != '/' )
				continue;

			// PATH_INFO = whatever that comes after *.cgi/
			if ( pos + ext.size() < path.size() )
				request.pathInfo = path.substr( pos + ext.size() );
			else
				request.pathInfo = "";

			// PATH_TRANSLATED = root + PATH_INFO
			request.pathTranslated = loc.root;
			if ( !request.pathInfo.empty() && request.pathInfo[ 0 ] == '/' )
				request.pathTranslated += request.pathInfo;
			else if ( !request.pathInfo.empty() )
				request.pathTranslated += "/" + request.pathInfo;

			// Adjust request.path
			path = path.substr( 0, pos + ext.size() );

			return;
		}
	}
}

static bool parseRequestLine( http::Request &req, const char *data, size_t &pos, size_t headerEnd ) {
	size_t lineEnd = std::string( data ).find( "\r\n", pos );
	if ( lineEnd == std::string::npos )
		return false;

	std::string requestLine( data + pos, lineEnd - pos );
	pos = lineEnd + 2;

	size_t posM = requestLine.find( ' ' );
	size_t posP = requestLine.find( ' ', posM + 1 );
	if ( posM == std::string::npos || posP == std::string::npos )
		return false;

	req.method = requestLine.substr( 0, posM );
	req.path = requestLine.substr( posM + 1, posP - posM - 1 );
	req.serverProtocol = requestLine.substr( posP + 1 );

	return true;
}

static void parseRequestHeaders( http::Request &req, const char *data, size_t &pos, size_t headerEnd ) {
	while ( pos < headerEnd ) {
		size_t lineEnd = std::string( data ).find( "\r\n", pos );
		if ( lineEnd == std::string::npos || lineEnd == pos )
			break;

		std::string line( data + pos, lineEnd - pos );
		pos = lineEnd + 2;

		size_t colon = line.find( ':' );
		if ( colon == std::string::npos )
			continue;

		std::string key = ft_strtrim( line.substr( 0, colon ) );
		std::string value = ft_strtrim( line.substr( colon + 1 ) );
		req.headers[ key ] = value;
	}
}
static bool parseRequestBody( http::Request &req, const std::string &buffer, const char *data, size_t bufferSize,
                              size_t bodyStart, Client &client ) {
	if ( req.headers.count( "Transfer-Encoding" ) && req.headers[ "Transfer-Encoding" ] == "chunked" ) {
		size_t bpos = bodyStart;
		std::string decoded;


		while ( true ) {
			size_t lineEnd = buffer.find( "\r\n", bpos );
			if ( lineEnd == std::string::npos ) {
				// std::cout << "HERE\n";
				// std::cout << buffer.substr(0, bpos);
				// sleep(2);
				return false;
			}
			
			std::string sizeStr( data + bpos, lineEnd - bpos );
			size_t chunkSize = strtoul( sizeStr.c_str(), NULL, 16 );
			if ( chunkSize == 0 ) {
				std::cout << "HERE2\n";
				
				break;
			}
			if ( bpos + chunkSize + 2 > bufferSize ) {
				// std::cout << "BPOS " << bpos << std::endl;
				client.setState( PARSE_INCOMPLETE );
				return false;
			}

			bpos = lineEnd + 2;
			decoded.append( data + bpos, chunkSize );
			bpos += chunkSize + 2;
		}
		req.body.append(decoded);
	} else if ( req.headers.count( "Content-Length" ) ) {
		size_t len = strtoul( req.headers[ "Content-Length" ].c_str(), NULL, 10 );
		if ( bodyStart + len <= bufferSize )
			req.body.assign( data + bodyStart, len );
		else {
			std::cout << "SETTING 5 on parseRequestBody second\n";
			client.setState( PARSE_INCOMPLETE );
			return false;
		}
	} else {
		req.body.clear();
	}
	return true;
}

bool http::ClientEventProcessor::parseRequestData( Client &client, const ServerConfig &serverInfo ) {
	
	std::string &readBuffer = client.getReadBuffer();
	size_t bufferSize = readBuffer.size();

	size_t headerEnd = readBuffer.find( "\r\n\r\n" );
	if ( headerEnd == std::string::npos ) {
		client.setState( PARSE_INCOMPLETE );
		return false;
	}

	const char *data = readBuffer.c_str();
	size_t pos = 0;
	http::Request &clientRequest = client.getRequest();

	if ( !parseRequestLine( clientRequest, data, pos, headerEnd ) ) {
		client.setState( PARSE_INCOMPLETE );

		return false;
	}

	parseRequestQueries( clientRequest );
	parsePath( clientRequest, serverInfo );

	parseRequestHeaders( clientRequest, data, pos, headerEnd );

	size_t bodyStart = headerEnd + 4;
	if ( !parseRequestBody( clientRequest, readBuffer, data, bufferSize, bodyStart, client ) )
		return false;

	client.getResponse() = Response( clientRequest );
	ensureSessionId( client );
	Logs::log( LOGS_INFO, "Client: " + ft_to_string( client.getFd() ) + " Made a Request" );
	client.setState( PARSE_OK );
	std::cout << clientRequest.method << " " << clientRequest.path << " " << clientRequest.serverProtocol << std::endl;
	printHttpHeaders( clientRequest.headers );
	std::cout << "client.getState()" << client.getState() << std::endl;

	return true;
}

// //!Before split
// bool http::ClientEventProcessor::parseRequestData( Client &client, const ServerConfig &serverInfo ) {

// 	std::string &readBuffer = client.getReadBuffer();
// 	size_t bufferSize = readBuffer.size();

// 	size_t headerEnd = readBuffer.find( "\r\n\r\n" );
// 	if ( headerEnd == std::string::npos ) {
// 		client.setState( PARSE_INCOMPLETE );
// 		return false;
// 	}

// 	const char *data = readBuffer.c_str();
// 	size_t pos = 0;

// 	// Request Line
// 	size_t lineEnd = readBuffer.find( "\r\n", pos );
// 	if ( lineEnd == std::string::npos ) {
// 		client.setState( PARSE_INCOMPLETE );
// 		return false;
// 	}

// 	std::string requestLine( data + pos, lineEnd - pos );
// 	pos = lineEnd + 2;

// 	size_t posM = requestLine.find( ' ' );
// 	size_t posP = requestLine.find( ' ', posM + 1 );
// 	if ( posM == std::string::npos || posP == std::string::npos ) {
// 		client.setState( PARSE_INCOMPLETE );
// 		return false;
// 	}

// 	http::Request &clientRequest = client.getRequest();
// 	clientRequest.method = requestLine.substr( 0, posM );
// 	clientRequest.path = requestLine.substr( posM + 1, posP - posM - 1 );
// 	clientRequest.serverProtocol = requestLine.substr( posP + 1 );
// 	parseRequestQueries( clientRequest );
// 	parsePath( clientRequest, serverInfo ); // ! Still thinking about this should be only when cgi is called
// 	// End first line
// 	while ( pos < headerEnd ) {
// 		size_t lineEnd2 = readBuffer.find( "\r\n", pos );
// 		if ( lineEnd2 == std::string::npos || lineEnd2 == pos )
// 			break;

// 		std::string line( data + pos, lineEnd2 - pos );
// 		pos = lineEnd2 + 2;

// 		size_t colon = line.find( ':' );
// 		if ( colon == std::string::npos )
// 			continue;
// 		std::string key = ft_strtrim( line.substr( 0, colon ) );
// 		std::string value = ft_strtrim( line.substr( colon + 1 ) );
// 		clientRequest.headers[ key ] = value;
// 	}
// 	size_t bodyStart = headerEnd + 4;
// 	if ( clientRequest.headers.count( "Transfer-Encoding" ) &&
// 	     clientRequest.headers[ "Transfer-Encoding" ] == "chunked" ) {
// 		size_t bpos = bodyStart;
// 		std::string decoded;
// 		while ( true ) {
// 			size_t lineEnd = readBuffer.find( "\r\n", bpos );
// 			if ( lineEnd == std::string::npos )
// 				break;

// 			std::string sizeStr( data + bpos, lineEnd - bpos );
// 			size_t chunkSize = strtoul( sizeStr.c_str(), NULL, 16 );
// 			if ( chunkSize == 0 )
// 				break;
// 			if ( bpos + chunkSize + 2 > bufferSize ) {
// 				client.setState( PARSE_INCOMPLETE );
// 				return false;
// 			}

// 			bpos = lineEnd + 2;
// 			decoded.append( data + bpos, chunkSize );
// 			bpos += chunkSize + 2;
// 		}
// 		clientRequest.body = decoded;
// 	} else if ( clientRequest.headers.count( "Content-Length" ) ) {
// 		size_t len = strtoul( clientRequest.headers[ "Content-Length" ].c_str(), NULL, 10 );
// 		if ( bodyStart + len <= bufferSize ) {
// 			clientRequest.body.assign( data + bodyStart, len );
// 		} else {
// 			client.setState( PARSE_INCOMPLETE );
// 			return false;
// 		}
// 	} else
// 		clientRequest.body.clear();

// 	client.getResponse() = Response( clientRequest );
// 	ensureSessionId( client );
// 	Logs::log( LOGS_INFO, "Client: " + ft_to_string( client.getFd() ) + " Made a Request" );
// 	client.setState( PARSE_OK );

// 	return true;
// }

//! My version before edits almost 2 Minutes of waiting
// bool http::ClientEventProcessor::parseRequestData( Client &client, const ServerConfig &serverInfo ) {
// 	size_t index = client.getReadBuffer().find( "\r\n\r\n" );

// 	if ( index == std::string::npos )
// 		return false;

// 	http::Request &clientRequest = client.getRequest();
// 	std::istringstream requestStream( client.getReadBuffer() );
// 	std::string line;

// 	if ( !std::getline( requestStream, line ) ) {
// 		client.setState( PARSE_INCOMPLETE );
// 		return false;
// 	}
// 	std::istringstream firstLine( line );

// 	firstLine >> clientRequest.method >> clientRequest.path >> clientRequest.serverProtocol;

// parseRequestQueries( clientRequest );
// 	parsePath( clientRequest, serverInfo );
// 	parseRequestHeaders( clientRequest, requestStream, line );

// 	size_t contentLength = 0;
// 	if ( clientRequest.headers.count( "Content-Length" ) ) {
// 		contentLength = std::strtoul( clientRequest.headers[ "Content-Length" ].c_str(), NULL, 10 );
// 		if ( contentLength > ( serverInfo.max_body_size ) ) {
// 			client._bytesToDiscard = contentLength;
// 			client._discardingBody = true;

// 			Logs::log( LOGS_ERROR, "Body size it's above size allowed " + ft_to_string( client.getFd() ) );
// 			return false;
// 		}
// 	}

// 	size_t bodyStart = client.getReadBuffer().find( "\r\n\r\n" ) + 4;
// 	std::string rawBody = client.getReadBuffer().substr( bodyStart );
// 	if ( clientRequest.headers.count( "Transfer-Encoding" ) &&
// 	     clientRequest.headers[ "Transfer-Encoding" ] == "chunked" ) {
// 		clientRequest.body = decodeChunked( rawBody );
// 	} else {
// 		clientRequest.body = rawBody;
// 	}

// 	bool isChunked =
// 	    clientRequest.headers.count( "Transfer-Encoding" ) && clientRequest.headers[ "Transfer-Encoding" ] == "chunked";

// 	if ( !isChunked && clientRequest.headers.count( "Content-Length" ) ) {
// 		size_t contentLength = std::strtoul( clientRequest.headers[ "Content-Length" ].c_str(), NULL, 10 );
// 		if ( clientRequest.body.size() < contentLength ) {
// 			client.setState( PARSE_INCOMPLETE );
// 			return false;
// 		}
// 	}

// 	if ( isChunked ) {
// 		index = client.getReadBuffer().find( "0\r\n\r\n" );

// 		if ( index == std::string::npos ) {
// 			client.setState( PARSE_INCOMPLETE );
// 			return false;
// 		}
// 	}

// 	client.getResponse() = Response( clientRequest );
// 	ensureSessionId( client );
// 	Logs::log( LOGS_INFO, "Client: " + ft_to_string( client.getFd() ) + " Made a Request" );
// 	client.setState( PARSE_OK );

// 	return true;
// }
