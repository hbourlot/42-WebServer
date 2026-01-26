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

static bool parseRequestLine( http::Request &req, const std::string &readBuffer, size_t lineEnd ) {

	const char *data = readBuffer.c_str();

	std::string requestLine( data, lineEnd );

	size_t posM = requestLine.find( ' ' );
	size_t posP = requestLine.find( ' ', posM + 1 );
	if ( posM == std::string::npos || posP == std::string::npos )
		return false;

	req.method = requestLine.substr( 0, posM );
	req.path = requestLine.substr( posM + 1, posP - posM - 1 );
	req.serverProtocol = requestLine.substr( posP + 1 );

	return true;
}

static void parseRequestHeaders( http::Request &req, const std::string &readBuffer, size_t headerEnd ) {
	const char *data = readBuffer.c_str();
	size_t pos = 0;
	while ( pos < headerEnd ) {
		size_t lineEnd = readBuffer.find( "\r\n", pos );
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

static bool parseRequestBody( http::Request &req, const std::string &readBuffer, Client &client ) {
	size_t bpos = 0;
	std::string decoded;
	const size_t bufferSize = readBuffer.size();

	if ( req.headers.count( "Transfer-Encoding" ) && req.headers[ "Transfer-Encoding" ] == "chunked" ) {

		while ( true ) {
			size_t lineEnd = readBuffer.find( "\r\n", bpos );
			if ( lineEnd == std::string::npos )
				return false;

			std::string sizeStr = readBuffer.substr( bpos, lineEnd - bpos );
			size_t chunkSize = strtoul( sizeStr.c_str(), NULL, 16 );

			bpos = lineEnd + 2;

			if ( chunkSize == 0 ) {
				if ( bufferSize < bpos + 2 )
					return false;
				if ( readBuffer.substr( bpos, 2 ) != "\r\n" )
					return false;
				bpos += 2;
				break;
			}

			if ( bufferSize < bpos + chunkSize + 2 )
				return false;

			decoded.append( readBuffer, bpos, chunkSize );

			if ( readBuffer.substr( bpos + chunkSize, 2 ) != "\r\n" )
				return false;

			bpos += chunkSize + 2;
		}

		req.body = decoded;
	} else if ( req.headers.count( "Content-Length" ) ) {
		size_t len = strtoul( req.headers[ "Content-Length" ].c_str(), NULL, 10 );
		if ( len > bufferSize )
			return false;
		req.body.assign( readBuffer, 0, len );
	} else {
		req.body.clear();
	}

	return true;
}

bool http::ClientEventProcessor::parseRequestData( Client &client, const ServerConfig &serverInfo ) {
	
	std::string &readBuffer = client.getReadBuffer();
	http::Request &clientRequest = client.getRequest();

	if ( client._requestPhase == START ) {
		size_t lineEnd = readBuffer.find( "\r\n" );
		if ( lineEnd == std::string::npos ) {
			client.setState( PARSE_INCOMPLETE );
			return false;
		}

		if ( !parseRequestLine( clientRequest, readBuffer, lineEnd ) ) {
			client.setState( PARSE_INCOMPLETE );
			return false;
		}

		parseRequestQueries( clientRequest );
		parsePath( clientRequest, serverInfo );
		readBuffer.erase( 0, lineEnd + 2 );
		client._requestPhase = HEADER;
	}

	if ( client._requestPhase == HEADER ) {
		size_t headerEnd = readBuffer.find( "\r\n\r\n" );
		if ( headerEnd == std::string::npos ) {
			client.setState( PARSE_INCOMPLETE );
			return false;
		}

		parseRequestHeaders( clientRequest, readBuffer, headerEnd );
		readBuffer.erase( 0, headerEnd + 4 );
		client._requestPhase = BODY;
	}

	if ( client._requestPhase == BODY ) {

		size_t bodyStart = 0;
		if ( !parseRequestBody( clientRequest, readBuffer, client ) ) {
			client.setState( PARSE_INCOMPLETE );
			return false;
		}

		readBuffer.erase();
		client._requestPhase = FINISHED;
	}

	if ( client._requestPhase == FINISHED ) {

		client.getResponse() = Response( clientRequest );
		ensureSessionId( client );
		Logs::log( LOGS_INFO, "Client: " + ft_to_string( client.getFd() ) + " Made a Request" );
		client.setState( PARSE_OK );
		client._requestPhase = START;
		return true;
	}
}
