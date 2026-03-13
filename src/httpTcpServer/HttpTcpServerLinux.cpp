#include "httpTcpServer/HttpTcpServerLinux.hpp"


namespace http {

	TcpServer::TcpServer( const ServerConfig &server, int portIndex	)
		: _serverInfo( server ), _serverSocket(), _socketAddress_len( sizeof( sockaddr_in ) ) {
		port = portIndex;
		std::string msg( "CREATED SERVER " );
		msg = msg + _serverInfo.host + ":";
		msg += ft_to_string( port);
		Logs::log( LOGS_INFO, msg );
	}

	TcpServer::~TcpServer() {
		close( _serverSocket );
	}

	int TcpServer::initializeServer() {
				
		// Creates a server socket (IPv4, TCP, 0) (domain, type, protocol);
		_serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
		if ( _serverSocket < 0 ) {
			throw TcpServerException( "Cannot create socket" );
			return -1;
		}

		// Set listening socket to non-blocking mode
		
		// For inactivate the time wait from OS that block bind again
		int opt = 1;
		if ( setsockopt( _serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0 ) {
			Logs::log( LOGS_ERROR, "setsockopt failed" );
			close( _serverSocket );
			exit( EXIT_FAILURE );
		}

		fcntl( _serverSocket, F_SETFL, fcntl( _serverSocket, F_GETFL, 0 ) | O_NONBLOCK );
		
		struct sockaddr_in socketAddress;

		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = htons( port );
		// Convert host string to binary form
		if ( _serverInfo.host.empty() || _serverInfo.host == "0.0.0.0" ) {
			socketAddress.sin_addr.s_addr = INADDR_ANY;
		} else {
			if ( inet_pton( AF_INET, _serverInfo.host.c_str(), &socketAddress.sin_addr ) <= 0 ) {
				throw TcpServerException( "Invalid IP address format" );
				return -1;
			}
		}

		// Associate socket with a specific IP addr and Port number (sockfd,
		if ( bind( _serverSocket, (sockaddr *)&socketAddress, _socketAddress_len ) < 0 ) {
			// perror( "bind" );
			throw TcpServerException( "Cannot bind socket to address" );
			return -1;
		}

		_socketAddressMap[_serverSocket] = socketAddress;

		return 0;
	}

	void TcpServer::startListen() {
		int listen_fd;

		listen_fd = listen( _serverSocket, SOMAXCONN );

		if ( listen_fd < 0 ) {
			throw TcpServerException( "Socket Listen failed" );
		}

		std::ostringstream ss; // Output string stream for logging'
		ss << "Listening on ADDRESS: " << inet_ntoa( _socketAddressMap[_serverSocket].sin_addr )
		   << " PORT: " << ntohs( _socketAddressMap[_serverSocket].sin_port );
		Logs::log( LOGS_INFO, ss.str() );
	}

	int TcpServer::startServer() {

		try {
			initializeServer();
			startListen();
		} catch ( const TcpServerException &e ) {
			std::cerr << "Error while starting to listen => " << e.what() << std::endl;
			close( _serverSocket );
			return -1;
		}

		_serverPOLLFD.fd = _serverSocket;
		_serverPOLLFD.events = POLLIN; // any readable data available
		_serverPOLLFD.revents = 0;

		return 0;
	}

	void TcpServer::setSocketAddress( SocketFD fd, sockaddr_in socketAddress ) {
		_socketAddressMap[fd] = socketAddress;
	}

	// GETTERS

	std::vector< pollfd > &TcpServer::getVectorPollFds() {
		return _fds;
	};

	ServerConfig &TcpServer::getServerInfo() {
		return _serverInfo;
	}

	std::map< SocketFD, sockaddr_in > &TcpServer::getSocketAddressRef() {
		return _socketAddressMap;
	}

	pollfd &TcpServer::getServerPOLLFD() {
		return _serverPOLLFD;
	}

} // namespace http