#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <sys/poll.h>
#include <unistd.h>

READ_STATUS
http::TcpServer::readRequest( int index ) {
	const size_t CLIENT_MAX_BODY_SIZE = _serverInfo.maxRequest * 1024 * 1024;
	char buffer[ BUFFER_SIZE + 1 ] = { 0 };

	int fd = _fds[ index ].fd;

	Client *client = _clientManager.getClient( fd );

	if ( !client )
		return READ_CLIENT_NOT_FOUND;

	ssize_t bytesReceived = read( fd, buffer, BUFFER_SIZE );
	if ( bytesReceived <= 0 ) {
		if ( bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) {
			Logs::log( ERROR, "Error: read()" );
			return READ_ERROR;
		}
		return READ_EMPTY;
	}

	client->appendToReadBuffer( std::string( buffer, bytesReceived ) );

	return READ_SUCCESS;
};
