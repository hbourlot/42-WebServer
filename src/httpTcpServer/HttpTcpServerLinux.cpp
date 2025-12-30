#include "Client/ClientEventProcessor.hpp"
#include "httpTcpServer/HttpStructs.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

#include <arpa/inet.h>
#include <cstddef>
#include <exception>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static void setSocketAddr( sockaddr_in &socketAddress, int domain, int s_addr, int _port ) {
	socketAddress.sin_family = domain;
	socketAddress.sin_addr.s_addr = s_addr;  // can replace this with a specific IP address if needed
	socketAddress.sin_port = htons( _port ); // Converts 16-bit integer in host byte order
}

namespace http {

	TcpServer::TcpServer( ServerConfig server )
	    : _serverInfo( server ), _serverSocket(), _socketAddress_len( sizeof( sockaddr_in ) ) {
		std::string msg( "CREATED SERVER " );
		msg = msg + _serverInfo.host + ":";
		msg += to_str( _serverInfo.port );
		Logs::log( LOGS_INFO, msg );
	}

	TcpServer::~TcpServer() {
		close( _serverSocket );
		// close(_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

	std::vector< pollfd > &TcpServer::getVectorPollFds() {
		return _fds;
	};

	int TcpServer::startServer() {

		// Creates a server socket (IPv4, TCP, 0) (domain, type, protocol);
		_serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
		if ( _serverSocket < 0 ) {
			throw TcpServerException( "Cannot create socket" );
			return -1;
		}

		// Set listening socket to non-blocking mode
		fcntl( _serverSocket, F_SETFL, fcntl( _serverSocket, F_GETFL, 0 ) | O_NONBLOCK );

		// For inactivate the time wait from OS that block bind again
		int opt = 1;
		if ( setsockopt( _serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0 ) {
			Logs::log( LOGS_ERROR, "setsockopt failed" );
			close( _serverSocket );
			exit( EXIT_FAILURE );
		}

		struct sockaddr_in socketAddress;
		// Set the socket address struct
		setSocketAddr( socketAddress, AF_INET, INADDR_ANY, _serverInfo.port );

		// Associate socket with a specific IP addr and Port number (sockfd,
		// sockaddr *, addrlen)
		if ( bind( _serverSocket, (sockaddr *)&socketAddress, _socketAddress_len ) < 0 ) {
			perror( "bind" );
			throw TcpServerException( "Cannot bind socket to address" );
			return -1;
		}

		_socketAddressMap[ _serverSocket ] = socketAddress;
		return 0;
	}

	void TcpServer::startListen() {
		int listen_fd;

		// TODO: Need to implement a properly valid max number of padding \
		// TODO: connections
		listen_fd = listen( _serverSocket, 10 );

		if ( listen_fd < 0 ) {
			throw TcpServerException( "Socket Listen failed" );
		}

		std::ostringstream ss; // Output string stream for logging
		ss << "Listening on ADDRESS: " << inet_ntoa( _socketAddressMap[ _serverSocket ].sin_addr )
		   << " PORT: " << ntohs( _socketAddressMap[ _serverSocket ].sin_port );
		Logs::log( LOGS_INFO, ss.str() );
	}

	void TcpServer::acceptConnection() {

		SocketFD acceptSocket;
		struct pollfd client_pollfd;
		struct sockaddr_in socketAddress;

		// Checks the if theres readable data available (event)
		while ( _fds[ 0 ].revents & POLLIN ) {
			acceptSocket = accept( _serverSocket, (struct sockaddr *)&socketAddress, &_socketAddress_len );
			if ( acceptSocket < 0 ) {
				if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
					// Means no more connections to accept
					break;
				}
				Logs::logAcceptError( socketAddress );
				return;
			} else {

				// Set client socket to non-blocking
				fcntl( acceptSocket, F_SETFL, fcntl( acceptSocket, F_GETFL, 0 ) | O_NONBLOCK );

				client_pollfd.fd = acceptSocket;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;

				_fds.push_back( client_pollfd );
				_socketAddressMap[ acceptSocket ] = socketAddress;

				_clientManager.addClient( acceptSocket, *this );

				// std::cout << "----- Connection Accepted 🟩\n\n";
				std::string msg( "Connection Accepted 🟩 " );
				msg += to_str( client_pollfd.fd );
				Logs::log( LOGS_INFO, msg );
			}
		}
	}

	int TcpServer::runServer() {

		int timeOut = 3 * 60 * 1000;

		if ( startServer() )
			return -1;
		try {
			startListen();
		} catch ( const TcpServerException &e ) {
			std::cerr << "Error while starting to listen => " << e.what() << std::endl;
			close( _serverSocket );
			return -1;
		}

		pollfd listen_fd;
		listen_fd.fd = _serverSocket;
		listen_fd.events = POLLIN; // any readable data available
		listen_fd.revents = 0;
		_fds.push_back( listen_fd );

		runLoop( timeOut );
		shutDownServer();
		return 0;
	}

	void TcpServer::removeDeadConnections( ClientEventProcessor &processor, int &index ) {

		if ( _fds[ index ].revents & ( POLLHUP | POLLERR | POLLNVAL ) ) {
			SocketFD fd = _fds[ index ].fd;

			// Check if this is a CGI pipe fd - skip it (handled by processCgiOutput)
			if ( _cgiByFd.find( fd ) != _cgiByFd.end() ) {
				return; // CGI pipes are managed separately
			}

			// Clean up CGI resources if this is a client with active CGI
			Client *client = _clientManager.getClient( fd );
			if ( client && client->getCgiOutputFd() != -1 ) {
				// Find and cleanup the CGI
				std::map< int, http::Cgi * >::iterator it = _cgiByFd.find( client->getCgiOutputFd() );
				if ( it != _cgiByFd.end() ) {
					processor.cleanupCgi( it->second );
				}
			}

			if ( _socketAddressMap.count( fd ) )
				_socketAddressMap.erase( fd );

			std::string msg( "Closing Dead FD => " );
			msg += to_str( fd );

			Logs::log( LOGS_ERROR, msg );
			close( fd );

			_clientManager.removeClient( fd );

			_fds.erase( _fds.begin() + index );
			--index;
		}
	}

	void TcpServer::runLoop( int timeOut ) {
		ClientEventProcessor processor( *this );
		try {
			while ( true ) {
				// poll() waits for events on multiple file descriptors (like
				// sockets), enabling non-blocking I/O in servers.
				int ret = poll( _fds.data(), _fds.size(), timeOut );

				if ( ret < 0 ) {
					std::cerr << "poll() failed" << std::endl;
					shutDownServer();
				} else if ( ret == 0 ) {
					std::cerr << "poll() timeOut. Closing Server." << std::endl;
					shutDownServer();
					return;
				}

				// Checking for new connections
				acceptConnection();
				int status = 0;
				for ( int i = 1; i < _fds.size(); ++i ) {
					removeDeadConnections( processor, i );
					processor.processClientEvents( i );
				}
			}
		} catch ( const TcpServerException &e ) {
			std::cerr << "Error handling client connection => " << e.what() << std::endl;
		} catch ( const std::exception &e ) {
			std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
		}
	}

	void TcpServer::shutDownServer() {
		// Close all CGI pipes before shutting down
		cleanupAllCgis();

		for ( int i = 0; i < _fds.size(); ++i ) {
			close( _fds[ i ].fd );
			_fds.erase( _fds.begin() + i );
		}
	}

	void TcpServer::closeClientConnection( size_t index ) { // TODO: [] Need to remove Cgi if it hass
		SocketFD fd = _fds[ index ].fd;
		Client *client = _clientManager.getClient( fd );

		if ( client && client->getCgiPid() != -1 ) {
			std::map< int, http::Cgi * >::iterator it = _cgiByFd.find( fd );
			if ( it != _cgiByFd.end() ) {
				delete it->second;
				_cgiByFd.erase( it );
			}
		}

		std::string msg( "Closing FD => " );
		msg += to_str( fd );

		Logs::log( LOGS_ERROR, msg );

		close( fd );

		_socketAddressMap.erase( fd );
		_clientManager.removeClient( fd );
		_fds.erase( _fds.begin() + index );
	}

	void TcpServer::cleanupAllCgis() {
		for ( std::map< int, http::Cgi * >::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ++it ) {
			delete it->second; // Cgi destructor closes pipes
		}
		_cgiByFd.clear();
		Logs::log( LOGS_INFO, "Cleaned up all CGI processes" );
	}

} // namespace http