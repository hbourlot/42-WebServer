#include "httpTcpServer/Response.hpp"
#include <cstdlib>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

http::Response::Response() : _protocol( "HTTP/1.1" ) {
}

void http::Response::buildCgiResponse( const HttpStatusCode &status, const std::string &body,
                                       const ServerConfig &server ) {
	(void)server;

	size_t headerEnd = body.find( "\r\n\r\n" );
	std::string headers;
	std::string content;

	if ( headerEnd != std::string::npos ) {
		headers = body.substr( 0, headerEnd );
		content = body.substr( headerEnd + 4 ); // Skipping \r\n\r\n
	} else {
		// No headers found in body of Cgi
		content = body;
	}

	// Parse CGI headers
	std::istringstream headerStream( headers );
	std::string line;
	bool hasStatus = false;
	bool hasContentType = false;

	while ( std::getline( headerStream, line ) ) {
		if ( line.empty() || line == "\r" )
			break;

		// Remove \r if has
		if ( !line.empty() && line[ line.size() - 1 ] == '\r' )
			line.erase( line.size() - 1 );

		size_t colonPos = line.find( ':' );
		if ( colonPos == std::string::npos )
			continue;

		std::string key = line.substr( 0, colonPos );
		std::string value = line.substr( colonPos + 1 );

		// Trim leading spaces from value
		size_t start = value.find_first_not_of( " \t" );
		if ( start != std::string::npos )
			value = value.substr( start );

		// Checking Status header (CGI can override status)
		if ( key == "Status" ) {
			hasStatus = true;
			// Parse status code from value (e.g., "200 OK")
			_statusCode = value.substr( 0, 3 ); // First 3 chars are the code

			size_t spacePos = value.find( ' ' );
			if ( spacePos != std::string::npos )
				_statusMsg = value.substr( spacePos + 1 );
			else
				_statusMsg = "";
		} else {
			// Store other CGI headers (Content-Type, Location, etc.)
			if ( key == "Content-Type" )
				hasContentType = true;
			_headers[ key ] = value;
		}
	}

	// If CGI didn't provide Status, use the one passed as parameter
	if ( !hasStatus ) {
		_statusCode = status.code;
		_statusMsg = status.message;
	}

	// If CGI didn't provide Content-Type, set default
	if ( !hasContentType ) {
		_headers[ "Content-Type" ] = "text/html";
	}

	_body = content;

	setDefaultHeaders();
}

http::Response::Response( const http::Request &request ) : _protocol( request.serverProtocol ) {
	std::map< std::string, std::string >::const_iterator it = request.headers.find( "Connection" );

	if ( it != request.headers.end() ) {
		std::string val = it->second;
		_connectionType = std::make_pair( std::string( "Connection" ), it->second );
	} else {
		if ( _protocol == "HTTP/1.1" )
			_connectionType = std::make_pair( std::string( "Connection" ), std::string( "keep-alive" ) );
		else
			_connectionType = std::make_pair( std::string( "Connection" ), std::string( "close" ) );
	}

	std::map< std::string, std::string >::const_iterator itRange = request.headers.find( "Range" );

	if ( itRange != request.headers.end() )
		_range = std::make_pair( itRange->first, itRange->second );
}

http::Response &http::Response::operator=( const Response &other ) {

	if ( this != &other ) {
		this->_protocol = other._protocol;
		this->_connectionType = other._connectionType;
		this->_range = other._range;
		this->_statusCode = other._statusCode;
		this->_statusMsg = other._statusMsg;
		this->_body = other._body;
		this->_headers = other._headers;
	}

	return *this;
}

http::Response::~Response() {
}

//! Member Function

void http::Response::addToHeader( std::string key, std::string value ) {
	this->_headers[ key ] = value;
}

void http::Response::setDefaultHeaders() {
	addToHeader( "Date", dateString() );

	std::ostringstream oss;
	oss << _body.size();
	addToHeader( "Content-Length", oss.str() );
	if ( !_connectionType.first.empty() )
		addToHeader( _connectionType.first, _connectionType.second );
}

std::string http::Response::buildResponseString() {

	if ( _statusCode.empty() )
		return "";

	std::ostringstream responseString;
	responseString << _protocol + " " << _statusCode << " " << _statusMsg << "\r\n";

	std::map< std::string, std::string >::const_iterator it;
	for ( it = _headers.begin(); it != _headers.end(); ++it )
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << _body;

	return responseString.str();
}

const std::map< std::string, std::string > &http::Response::getHeaders() const {
	return ( _headers );
}

// Here function

void http::Response::buildResponse( const HttpStatusCode &status, const std::string &body ) {
	_statusCode = status.code;
	_statusMsg = status.message;
	_body = body;

	setDefaultHeaders();
}
static std::string createErrorBody( const HttpStatusCode &status ) {
	std::string html;

	html += "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n<title>Error Occurred</title>\n";
	html += "<style>\n";
	html +=
	    "body { font-family: Arial, sans-serif; text-align: center; padding-top: 10%; background-color: #f9f9f9; }\n";
	html += ".error-box { display: inline-block; border-radius: 10px; padding: 2em 3em; background: #fff; box-shadow: "
	        "0 2px 8px rgba(0,0,0,0.1); }\n";
	html += "h1 { margin-bottom: 0.5em; color: #d9534f; }\n";
	html += "p { color: #555; margin-bottom: 1.5em; }\n";
	html += "a.button { text-decoration: none; color: white; background: #007bff; padding: 0.7em 1.5em; border-radius: "
	        "5px; font-weight: bold; }\n";
	html += "a.button:hover { background: #0056b3; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<div class=\"error-box\">";
	html += "<h1>Error " + status.code + "</h1>\n";
	html += "<p>Sorry, " + status.message + "</p>\n";
	html += "<a href=\"/\" class=\"button\">Go Home</a>\n";
	html += "</div>\n";
	html += "</body>\n</html>\n";

	return html;
}

void http::Response::buildErrorResponse( const HttpStatusCode &status, const ServerConfig &server ) {
	std::map< int, std::string >::const_iterator it;
	it = server.errorPage.find( atoi( status.code.c_str() ) );
	if ( it != server.errorPage.end() ) {
		std::ifstream file( it->second.c_str() );
		if ( file.good() ) {
			buildFileResponse( status, it->second, server );
			return;
		}
	}
	buildResponse( status, createErrorBody( status ) );
	addToHeader( "Content-Type", "text/html" );
	if ( status.code.at( 0 ) == '4' || status.code.at( 0 ) == '5' )
		addToHeader( "Connection", "close" );
}
void http::Response::buildRedirect( const HttpStatusCode &status, const std::string &url ) {
	buildResponse( status, "" );
	addToHeader( "Location", url );
}

static bool parseRange( std::string &rangeValue, off_t &fileSize, off_t &start, off_t &end ) {
	std::string prefix = "bytes=";
	if ( rangeValue.compare( 0, prefix.size(), prefix ) != 0 )
		return ( false );

	std::string rangeSpec = rangeValue.substr( prefix.size() );
	size_t dash = rangeSpec.find( '-' );
	if ( dash == std::string::npos )
		return ( false );

	std::string startStr = rangeSpec.substr( 0, dash );
	std::string endStr = rangeSpec.substr( dash + 1 );

	start = 0;
	end = fileSize - 1;

	if ( !startStr.empty() )
		start = static_cast< off_t >( atoll( startStr.c_str() ) );
	if ( !endStr.empty() )
		end = static_cast< off_t >( atoll( endStr.c_str() ) );

	if ( start >= fileSize )
		return ( false );
	if ( end >= fileSize )
		end = fileSize - 1;
	const off_t MAX_CHUNK = 64 * 1024; // 16KB
	if ( end - start + 1 > MAX_CHUNK )
		end = start + MAX_CHUNK - 1;
	if ( start > end )
		return ( false );

	return ( true );
}

void http::Response::buildRangeResponse( const std::string &filePath, const ServerConfig &server, struct ::stat &st ) {
	off_t start;
	off_t end;

	if ( !parseRange( _range.second, st.st_size, start, end ) )
		return ( buildErrorResponse( HTTP_RANGE_NOT_SATISFIABLE, server ) );

	off_t diff = end - start + 1;

	std::ifstream file( filePath.c_str(), std::ios::binary );
	if ( !file.is_open() )
		return ( buildErrorResponse( HTTP_SERVER_ERR, server ) );

	file.seekg( start, std::ios::beg );

	std::ostringstream body;
	char buffer[ CHUNK_SIZE + 1 ] = { 0 };
	while ( diff > 0 && file.good() ) {
		std::streamsize toRead = std::min< off_t >( diff, CHUNK_SIZE );
		file.read( buffer, toRead );
		std::streamsize bytesRead = file.gcount();
		if ( bytesRead <= 0 )
			break;

		body.write( buffer, bytesRead );
		diff -= bytesRead;
	}
	file.close();

	buildResponse( HTTP_PARTIAL_CONTENT, body.str() );
	std::ostringstream rangeHeader;
	rangeHeader << "bytes " << start << "-" << end << "/" << st.st_size;
	if ( DEBUG )
		Logs::log( LOGS_INFO, "Request for range: " + rangeHeader.str() );
	addToHeader( "Content-Range", rangeHeader.str() );
	addToHeader( "Content-Type", getContentType( filePath ) );
	addToHeader( "Content-Length", ft_to_string( end - start + 1 ) );
}
void http::Response::buildFileResponse( const HttpStatusCode &status, const std::string &filePath,
                                        const ServerConfig &server ) {
	struct ::stat st;
	if ( ::stat( filePath.c_str(), &st ) != 0 )
		return ( buildErrorResponse( HTTP_NOT_FOUND, server ) );

	if ( !_range.first.empty() ) {
		buildRangeResponse( filePath, server, st );
		return;
	}

	std::string content = readFileContent( filePath );

	buildResponse( status, content );
	addToHeader( "Content-Type", getContentType( filePath ) );
}

std::string http::Response::readFileContent( const std::string &filePath ) {
	std::ifstream file( filePath.c_str() );

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

std::string http::Response::getContentType( const std::string &filePath ) {
	size_t dot = filePath.find_last_of( '.' );
	if ( dot == std::string::npos )
		return "application/octet-stream"; // generic Binary

	std::string ext = filePath.substr( dot + 1 );
	if ( ext == "html" || ext == "htm" )
		return "text/html";
	if ( ext == "css" )
		return "text/css";
	if ( ext == "png" )
		return "image/png";
	if ( ext == "jpg" || ext == "jpeg" )
		return "image/jpeg";
	if ( ext == "gif" )
		return "image/gif";
	if ( ext == "txt" )
		return "text/plain";
	if ( ext == "pdf" )
		return "application/pdf";
	if ( ext == "mp4" )
		return "video/mp4";
	if ( ext == "js" )
		return "text/javascript";
	return "application/octet-stream";
}

bool http::Response::shouldCloseConnection() {
	std::map< std::string, std::string >::const_iterator it = _headers.find( "Connection" );
	if ( it != _headers.end() ) {
		if ( it->second == "close" )
			return true;
		return false;
	}
	return ( false );
}