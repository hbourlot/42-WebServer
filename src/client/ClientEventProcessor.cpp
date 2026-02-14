#include "Client/ClientEventProcessor.hpp"
// #include "Http/Response.hpp"

http::ClientEventProcessor::ClientEventProcessor( std::vector< pollfd > &allFds, std::vector< TcpServer * > servers )
	: _allSockets( allFds ), _servers( servers ) {

	_serverSocketSize = _allSockets.size();
};

http::ClientEventProcessor::~ClientEventProcessor() {};

static void discardingBody( Client &client, pollfd &pfd ) {
	size_t available = client.getReadBuffer().size();
	size_t bytesToDiscard = client.getBytesToDiscard();

	if ( available >= bytesToDiscard ) {
		client.consumeReadBuffer( bytesToDiscard );
		client.setBytesToDiscard( 0 );
		client.setDiscardingBody( false );

		client.getResponse() = http::Response( client.getRequest() );
		ensureSessionId( client );
		client.setState( PARSE_TOO_LARGE );

		pfd.events &= ~POLLIN;
		pfd.events |= POLLOUT;
	} else {
		client.setBytesToDiscard( bytesToDiscard - available );
		client.clearReadBuffer();
	}
}

void http::ClientEventProcessor::run() {
	
	int timeOut = 1 * 10 * 1000; // 10s

	try {
		while ( true ) {
			int ret = poll( _allSockets.data(), _allSockets.size(), timeOut );

			if ( ret < 0 )
				std::cerr << "poll() failed" << std::endl;
			else if ( ret == 0 ) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				break;
			}

			// Checking for new Connections
			acceptConnections();
			for ( size_t i = _serverSocketSize; i < _allSockets.size(); ++i ) {
				bool erased = removeDeadConnections( i );
				if ( erased )
					continue;
				this->processClientEvents( i );
				checkIdleConnections( i );
			}
		}
	} catch ( ClientEventProcessorException &e ) {
		std::cerr << "Error handling client connection => " << e.what() << std::endl;
	} catch ( const std::exception &e ) {
		std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	}

	shutDownProcessor();
	return;
}

void http::ClientEventProcessor::acceptConnections() {

	SocketFD fd;
	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	for ( int i = 0; i < _serverSocketSize; ++i ) {
		while ( _allSockets[i].revents && POLLIN ) {
			unsigned int socketAddress_len = sizeof( sockaddr_in );
			fd = accept( _allSockets[i].fd, (struct sockaddr *)&socketAddress, &socketAddress_len );
			
			if ( fd < 0 ) {
				if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
					// Means no more connections to accept
					break;
				}
				Logs::logAcceptError( socketAddress );
				return;
			} else {

				// Set client socket to non-blocking
				fcntl( fd, F_SETFL, fcntl( fd, F_GETFL, 0 ) | O_NONBLOCK );

				client_pollfd.fd = fd;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;

				_allSockets.push_back( client_pollfd );

				_servers[i]->setSocketAddress( fd, socketAddress );
				_clientManager.addClient( fd, ( *_servers[i] ) );

				std::string msg( "Connection Accepted 🟩 " );
				msg += ft_to_string( client_pollfd.fd );
				Logs::log( LOGS_INFO, msg );
			}
		}
	}
}

bool http::ClientEventProcessor::removeDeadConnections( size_t &index ) {

	if ( _allSockets[index].revents & ( POLLHUP | POLLERR | POLLNVAL ) ) {
		// if (_allSockets[index].revents & (POLLERR | POLLNVAL)) {
		SocketFD fd = _allSockets[index].fd;

		// Check if this is a CGI pipe fd - skip it (handled by processCgiOutput)
		if ( _cgi_by_fd.find( fd ) != _cgi_by_fd.end() ) {
			return false; // CGI pipes are managed separately
		}

		// Clean up CGI resources if this is a client with active CGI
		Client *client = _clientManager.getClient( fd );
		if ( client && client->getCgiOutputFd() != -1 ) {
			// Find and cleanup the CGI
			std::map< int, http::Cgi * >::iterator it = _cgi_by_fd.find( client->getCgiOutputFd() );
			if ( it != _cgi_by_fd.end() ) {
				this->cleanupCgi( it->second );
			}
		}

		if ( client->getServer().getSocketAddressRef().count( fd ) )
			client->getServer().getSocketAddressRef().erase( fd );

		std::string msg( "Closing Dead FD => " );
		msg += ft_to_string( fd );

		Logs::log( LOGS_ERROR, msg );

		_clientManager.removeClient( fd );

		_allSockets.erase( _allSockets.begin() + index );
		// --index;
		close( fd );
		return true;
	}
	return false;
};

void http::ClientEventProcessor::checkIdleConnections( size_t index ) {

	const int MAX_IDLE_TICKS = 3000;

	SocketFD fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient( fd );

	if ( !client )
		return;

	if ( _allSockets[index].revents == POLLIN && client->getState() == READ_EMPTY ) {
		client->incrementIdleTicks();
		if ( client->getIdleTicks() >= MAX_IDLE_TICKS ) {
			closeClientConnection( index );
		}
		return;
	} else {
		client->resetIdleTicks();
	}
}

void http::ClientEventProcessor::closeClientConnection( size_t index ) {
	SocketFD fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient( fd );

	if ( client && client->getCgiPid() != -1 ) {
		std::map< int, http::Cgi * >::iterator it = _cgi_by_fd.find( fd );
		if ( it != _cgi_by_fd.end() ) {
			delete it->second;
			_cgi_by_fd.erase( it );
		}
	}

	std::string msg( "Closing FD => " );
	msg += ft_to_string( fd );

	Logs::log( LOGS_WARN, msg );

	if ( client ) {
		client->getServer().getSocketAddressRef().erase( fd );
	}
	_clientManager.removeClient( fd );
	_allSockets.erase( _allSockets.begin() + index );
	close( fd );
}

static void cleanupAllCgis( std::map< SocketFD, http::Cgi * > &cgis ) {

	for ( std::map< int, http::Cgi * >::iterator it = cgis.begin(); it != cgis.end(); ++it ) {
		it->second->killProcess();
		delete it->second; // Cgi destructor closes pipes
	}
	cgis.clear();
	Logs::log( LOGS_INFO, "Cleaned up all CGI processes" );
}

void http::ClientEventProcessor::shutDownProcessor() {
	Logs::log( LOGS_INFO, "===== Starting to shut down the Server =====" );

	// Close all CGI pipes before shutting down
	cleanupAllCgis( _cgi_by_fd );

	for ( size_t i = 0; i < _allSockets.size(); ++i ) {
		std::string msg = "Removing from poll vector at idx '" + ft_to_string( i );
		msg += "' fd='";
		msg += ft_to_string( _allSockets[i].fd );
		msg += "'";
		Logs::log( LOGS_INFO, msg );

		if ( _allSockets[i].fd != -1 )
			close( _allSockets[i].fd );
		_allSockets.erase( _allSockets.begin() + i );
		--i;
	}
	Logs::log( LOGS_INFO, "===== END =====" );
}

void http::ClientEventProcessor::processRead( pollfd &pfd, Client *client, Cgi *cgi ) {

	std::string &readBuffer = cgi ? cgi->getReadBuffer() : client->getReadBuffer();
	
	if ( !readFromSocket( pfd.fd, readBuffer, cgi ? cgi->getState() : client->getState() ) ) {
		return;
	}

	if ( cgi ) {
		return;
	}

	if ( client->getDiscardingBody() ) {
		discardingBody( *client, pfd );
		return;
	}

	if ( !parseRequestData( *client, client->getServer()._serverInfo ) ) {
		return;
	}

	pfd.events = POLLOUT; // Setting to POLL OUT
};

void http::ClientEventProcessor::processWrite( pollfd &pfd, Client *client, int index ) {

	std::map< SocketFD, Cgi * >::iterator it = _cgi_by_fd.find( client->getCgiOutputFd() );
	Cgi *cgi = nullptr;
	if ( it != _cgi_by_fd.end() ) {
		cgi = it->second;
	}

	if ( client->getState() == CGI_COMPLETED && cgi ) {

		std::string &readBuffer = cgi->getReadBuffer();

		if ( client->getWriteBuffer().empty() )
			client->getResponse().buildCgiResponse( HTTP_OK, readBuffer, client->getServer()._serverInfo );
		cleanupCgi( cgi );

	} else if ( client->getCgiPid() == -1 && client->getState() != CGI_COMPLETED ) {
		if ( !processRequest( *client ) )
			return;
	}

	if ( handleResponse( pfd, *client ) ) {
		this->closeClientConnection( index );
	}
};

bool http::ClientEventProcessor::readFromSocket( SocketFD fd, std::string &readBuffer, IN_OUT_STATE &state ) {

	char buffer[BUFFER_SIZE];
	int readCount = 0;
	bool dataReceived = false;

	// Read up to MAX_READS_PER_EVENT times per poll event
	while ( readCount < MAX_READS_PER_EVENT ) {

		std::memset( buffer, 0, BUFFER_SIZE );
		ssize_t bytesReceived = read( fd, buffer, BUFFER_SIZE - 1 );

		if ( bytesReceived > 0 ) {
			readBuffer.append( buffer, bytesReceived );
			dataReceived = true;
			readCount++;
			continue; // Try to read more data
		}

		if ( bytesReceived == 0 ) {
			break;
		}

		if ( bytesReceived == 0 && readCount == 0 ) {
			// Peer closed connection
			state = READ_EMPTY;
			return false;
		}

		// bytesReceived < 0
		if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
			// No more data available, this is normal
			break;
		}

		// Fatal error
		Logs::log( LOGS_ERROR, "Error: recv()" );
		state = READ_ERROR;
		return false;
	}

	if ( dataReceived ) {
		state = READ_SUCCESS;
		return true;
	}

	state = READ_EMPTY;
	return false;
}

bool http::ClientEventProcessor::processRequest( Client &client ) {

	IN_OUT_STATE state = client.getState();

	ServerConfig &serverInfo = client.getServer()._serverInfo;
	// Handle error states first (build error responses)
	if ( state != PARSE_OK ) {
		this->buildErrorResponse( client, state );
		return true;
	}
	// Handling SuccessfulRequest - from here
	VALIDATION_STATUS validationStatus;
	if ( this->handleRouteValidation( client, validationStatus ) )
		return true;

	if ( validationStatus == VALID_IS_CGI )
		Router::routeCgiRequest( client, serverInfo, *client.getRequest().matchLocation, *this );
	else
		Router::routeStaticRequest( client, serverInfo, *client.getRequest().matchLocation );

	return true;
}

bool http::ClientEventProcessor::buildErrorResponse( Client &client, IN_OUT_STATE state ) {
	http::Response &response = client.getResponse();

	switch ( state ) {
	case READ_ERROR:
		response.buildErrorResponse( HTTP_SERVER_ERR, client.getServer()._serverInfo );
		return true;
	case READ_EMPTY:
		response.buildErrorResponse( HTTP_BAD_REQ, client.getServer()._serverInfo );
		return true;
	case PARSE_TOO_LARGE:
		response.buildErrorResponse( HTTP_PAYLOAD, client.getServer()._serverInfo );
		return true;
	default:
		response.buildErrorResponse( HTTP_SERVER_ERR, client.getServer()._serverInfo );
		return true;
	}
}

bool http::ClientEventProcessor::handleRouteValidation( Client &client, VALIDATION_STATUS &validationStatus ) {
	http::Response &response = client.getResponse();
	ServerConfig &serverInfo = client.getServer()._serverInfo;
	validationStatus = Router::validateRequest( client );

	switch ( validationStatus ) {

	case VALID_IS_CGI:
		return false;

	case VALID_OK:
		return false; // Continue to routing

	case VALID_NOT_FOUND:
		response.buildErrorResponse( HTTP_NOT_FOUND, serverInfo );
		return true;

	case VALID_REDIRECT_REQUIRED:
		response.buildRedirect( HTTP_MOVED, client.getRequest().matchLocation->redirection );
		return true;

	case VALID_METHOD_NOT_ALLOWED:
		response.buildErrorResponse( HTTP_FORBID_METHOD, serverInfo );
		return true;
	case VALID_FORBIDDEN:
		response.buildErrorResponse( HTTP_FORBID, serverInfo );
		return true;

	default:
		response.buildErrorResponse( HTTP_SERVER_ERR, serverInfo );
		return true;
	}
}

void http::ClientEventProcessor::processClientEvents( int index ) {

	int fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient( fd );

	std::map< int, Cgi * >::iterator it = _cgi_by_fd.find( fd );
	Cgi *cgi = ( it != _cgi_by_fd.end() ) ? it->second : nullptr;

	if ( cgi )
		client = cgi->getClient();

	// Regular client socket handling
	if ( _allSockets[index].revents & POLLIN ) {
		processRead( _allSockets[index], client, cgi );
	}

	if ( _allSockets[index].revents & POLLOUT ) {
		processWrite( _allSockets[index], client, index );
	}

	if ( cgi && hasCgiFinished( cgi ) ) {
		client->setState( CGI_COMPLETED );
	}
}

bool http::ClientEventProcessor::handleResponse( pollfd &pfd, Client &client ) {
	SocketFD clientFd = client.getFd();

	// Build response if write buffer is emptysendResponse
	if ( client.getWriteBuffer().empty() )
		client.appendToWriteBuffer( client.getResponse().buildResponseString() );

	std::string &writeBuffer = client.getWriteBuffer();

	if ( writeBuffer.empty() )
		return 0;
	if ( sendResponse( pfd, client ) ) {
		return ( 1 );
	}

	// Check if all data was sent
	if ( writeBuffer.empty() ) {
		std::string msg( "Server Response sent to client fd='" );
		msg += ft_to_string( clientFd ) + "' sessionID: " + client.getSessionId();
		if ( DEBUG ) {
			msg += " ";
			msg += client.getRequest().path;
		}
		Logs::log( LOGS_INFO, msg );

		if ( client.getResponse().shouldCloseConnection() ) {
			client.getServer()._clientManager.resetClientState( clientFd );

			return 1; // Close connection
		}
		client.getServer()._clientManager.resetClientState( clientFd );
		pfd.events = POLLIN; // Reset to read for next request
		return 0;
	}

	// Still have data to send, keep POLLOUT active
	pfd.events |= POLLOUT;
	return 0; // Continue sending in next poll event
}

bool http::ClientEventProcessor::sendResponse( pollfd &pfd, Client &client ) {
	SocketFD clientFd = client.getFd();

	int sendCount = 0;
	std::string &writeBuffer = client.getWriteBuffer();

	// Send up to MAX_SENDS_PER_EVENT times per poll event
	while ( sendCount < MAX_SENDS_PER_EVENT && !writeBuffer.empty() ) {
		ssize_t bytesSent = send( clientFd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL );

		if ( bytesSent < 0 ) {
			if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
				// Socket buffer full, will continue later
				pfd.events = POLLOUT;
				return 0; // Keep connection alive, continue sending later
			}

			if ( errno == EPIPE )
				Logs::log( LOGS_ERROR, "Client disconnected before response." );
			else
				Logs::log( LOGS_ERROR, "Error sending response to client." );
			return 1; // Close connection
		}

		if ( bytesSent == 0 )
			break; // Should not happen with send(), but handle gracefully

		writeBuffer.erase( 0, bytesSent );
		sendCount++;
	}
	return ( 0 );
}