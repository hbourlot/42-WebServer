#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <sys/poll.h>
#include <unistd.h>



REQUEST_STATUS
http::TcpServer::readRequest( int index ) {
	const size_t CLIENT_MAX_BODY_SIZE = _serverInfo.maxRequest * 1024 * 1024;
	char buffer[ BUFFER_SIZE + 1 ] = { 0 };

	int fd = _fds[ index ].fd;

	Client *client = _clientManager.getClient( fd );

	if ( !client )
		return CLIENT_NOT_FOUND;

	ssize_t bytesReceived = read( fd, buffer, BUFFER_SIZE );
	if ( bytesReceived <= 0 ) {
		if ( bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) {
			Logs::log(ERROR, "Error: read()");
			return MAIN_FUNCTIONALITY_PROBLEMS;
		}
		return REQUEST_EMPTY;
	}

	client->appendToReadBuffer( std::string( buffer, bytesReceived ) );

	return REQUEST_READ_SUCCESSFULLY;
};

bool http::TcpServer::readRequest( int index ) {

	const size_t CLIENT_MAX_BODY_SIZE = _serverInfo.maxRequest * 1024 * 1024; // 10MB
	char buffer[ BUFFER_SIZE + 1 ] = { 0 };
	int fd = _fds[ index ].fd;

	bool shouldPollOutStatus = false;
	bool shouldCloseStatus = false;

	Client *client = _clientManager.getClient( fd );
	// if ( !client ) {
	// 	std::cerr << "Error: Client not found for fd " << fd << std::endl;
	// 	client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_BAD_REQ );
	// 	_fds[ index ].events |= POLLOUT;
	// 	return true; // Close connection
	// }

	ssize_t bytesReceived = read( fd, buffer, BUFFER_SIZE );
	if ( bytesReceived <= 0 ) {
		if ( bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK )
			std::cerr << "Error: read()\n";

		client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_BAD_REQ );
		shouldPollOutStatus = true;
		shouldCloseStatus = true;
	} else {
		client->appendToReadBuffer( std::string( buffer, bytesReceived ) );

		PARSE_STATUS status =
		    parseRequest( client->getRequest(), client->getReadBuffer(), _serverInfo, CLIENT_MAX_BODY_SIZE );

		if ( status == PARSE_TOO_LARGE ) {
			client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_PAYLOAD );
			// setResponseError(HTTP_PAYLOAD);
			shouldPollOutStatus = true;
			shouldCloseStatus = true;
		}
		//! Need more lecture  says that its the max for each request

		if ( status == PARSE_OK ) {

			HttpHandler::handle( *client, _serverInfo );
			client->clearReadBuffer();
			shouldPollOutStatus = true;
			shouldCloseStatus = false;
		} else if ( status == PARSE_INCOMPLETE )
			return false;
	}

	if ( shouldPollOutStatus )
		_fds[ index ].events |= POLLOUT;

	return shouldCloseStatus;
}
