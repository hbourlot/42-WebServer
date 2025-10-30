#include "Client/ClientEventProcessor.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

// bool http::TcpServer::handleCgiResponse( pollfd &socket ) {
// 	Client *client = _clientManager.getClient( socket.fd );

// 	std::cout << "[DEBUG] FD: " << socket.fd << " | revents: " << socket.revents << " | events: " << socket.events
// 	          << std::endl;
// 	if ( ( socket.revents & POLLIN ) && _cgiFdMap.count( socket.fd ) ) {
// 		std::cout << "FD ON HANDLE => " << socket.fd << std::endl;
// 		Cgi *cgi = _cgiFdMap[ socket.fd ];
// 		cgi->readCgiOutput();

// 		client->getResponse() = ResponseBuilder::buildResponse( HTTP_OK, cgi->getBody() );

// 		std::cout << cgi->getBody();
// 		sendResponse( socket );
// 		_cgiFdMap.erase( socket.fd );
// 		return true;
// 	}
// 	return false;
// }

void http::TcpServer::closeClientConnection( size_t index ) {
	SocketFD fd = _fds[ index ].fd;

	std::string msg( "Closing FD => " );
	msg += to_str( fd );

	Logs::log( ERROR, msg );

	close( fd );
	_socketAddressMap.erase( fd );
	_clientManager.removeClient( fd );
	_fds.erase( _fds.begin() + index );
}

void http::TcpServer::processClientEvents( ClientEventProcessor &processor ) {

	for ( size_t i = 1; i < _fds.size(); ++i ) {
		Client *client = _clientManager.getClient( _fds[ i ].fd );
		if ( !client ) {
			closeClientConnection( i );
			continue;
		}

		if ( _fds[ i ].revents & POLLIN ) {
			processor.processRead( _fds[ i ], *client );
		}

		if ( _fds[ i ].revents & POLLOUT ) {
			processor.processWrite( _fds[ i ], *client );
		}
	}
};