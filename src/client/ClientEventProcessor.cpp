#include "Client/ClientEventProcessor.hpp"

http::ClientEventProcessor::ClientEventProcessor( TcpServer &server ) : _server( server ) {};

http::ClientEventProcessor::~ClientEventProcessor() {};

void http::ClientEventProcessor::processRead( pollfd &pfd, Client &client ) {

	// _clientManager.IsCgi(fd);
	// * Client client = _clientManaget.getClientFromCgi(fd);

	// cgi.read

	if ( !readFromSocket( client ) )
		return;
	if ( !parseRequestData( client, _server._serverInfo ) )
		return;
	pfd.events |= POLLOUT; // Setting to POLL OUT
};

void http::ClientEventProcessor::processWrite( pollfd &pfd, Client &client ) {

	// !! Need to check if it's a childProcess or a ClientSocket
	if ( !processRequest( client ) )
		return;
	std::cout << "In processWrite\n";
	sendResponse( pfd, client );
};

bool http::ClientEventProcessor::readFromSocket( Client &client ) {

	const size_t CLIENT_MAX_BODY_SIZE = _server._serverInfo.maxRequest * 1024 * 1024;
	const int MAX_READS_PER_EVENT = 3;
	char buffer[ BUFFER_SIZE ];
	int fd = client.getFd();
	int readCount = 0;
	bool dataReceived = false;

	// Read up to MAX_READS_PER_EVENT times per poll event
	while ( readCount < MAX_READS_PER_EVENT ) {
		ssize_t bytesReceived = recv( fd, buffer, BUFFER_SIZE, 0 );

		if ( bytesReceived > 0 ) {
			// Check if we exceed max body size
			if ( client.getReadBuffer().size() + bytesReceived > CLIENT_MAX_BODY_SIZE ) {
				client.setState( PARSE_TOO_LARGE );
				return false;
			}

			client.appendToReadBuffer( std::string( buffer, static_cast< size_t >( bytesReceived ) ) );
			dataReceived = true;
			readCount++;
			continue; // Try to read more data
		}

		if ( bytesReceived == 0 && readCount == 0 ) {
			// Peer closed connection
			client.setState( READ_EMPTY );
			return false;
		}

		// bytesReceived < 0
		if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
			// No more data available, this is normal
			break;
		}

		// Fatal error
		Logs::log( ERROR, "Error: recv()" );
		client.setState( READ_ERROR );
		return false;
	}

	if ( dataReceived ) {
		client.setState( READ_SUCCESS );
		return true;
	}

	client.setState( READ_EMPTY );
	return false;
}

void http::ClientEventProcessor::closeConnection( size_t index ) {
	_server.closeClientConnection( index );
}

bool http::ClientEventProcessor::processRequest( Client &client ) {
	CLIENT_STATE state = client.getState();

	// Handle error states first (build error responses)
	if ( state != PARSE_OK ) {
		return buildErrorResponse( client, state );
	}

	// Handle successful parse (validate & route)
	return handleSuccessfulRequest( client );
}

bool http::ClientEventProcessor::buildErrorResponse( Client &client, CLIENT_STATE state ) {
	httpResponse &response = client.getResponse();

	switch ( state ) {
	case READ_ERROR:
		response = ResponseBuilder::buildErrorResponse( HTTP_SERVER_ERR );
		return true;
	case READ_EMPTY:
		response = ResponseBuilder::buildErrorResponse( HTTP_BAD_REQ );
		return true;
	case PARSE_TOO_LARGE:
		response = ResponseBuilder::buildErrorResponse( HTTP_PAYLOAD );
		return true;
	default:
		response = ResponseBuilder::buildErrorResponse( HTTP_SERVER_ERR );
		return true;
	}
}

bool http::ClientEventProcessor::handleSuccessfulRequest( Client &client ) {

	// Validate the request (routing, permissions, etc.)
	if ( !handleRouteValidation( client ) )
		return false;

	executeRequest( client );

	return true;
}

bool http::ClientEventProcessor::handleRouteValidation( Client &client ) {
	httpResponse &response = client.getResponse();
	ServerConfig &serverInfo = _server._serverInfo;

	VALIDATION_STATUS validationStatus = HttpRouter::validateRequest( client, _server._serverInfo );

	switch ( validationStatus ) {

	case VALID_OK:
		return true; // Continue to routing

	case VALID_IS_CGI:
		return true;

	case VALID_NOT_FOUND:
		response =
		    ResponseBuilder::buildFileResponse( HTTP_NOT_FOUND, serverInfo.errorPage.at( 404 ), serverInfo, true );
		return false;

	case VALID_REDIRECT_REQUIRED:
		response = ResponseBuilder::buildRedirect( HTTP_MOVED, client.getRequest().urlMatchedLocation->redirection );
		return false;

	case VALID_METHOD_NOT_ALLOWED:
		response = ResponseBuilder::buildFileResponse( HTTP_FORBID_METHOD, DFL_405, serverInfo, true );
		return false;

	default:
		response = ResponseBuilder::buildErrorResponse( HTTP_SERVER_ERR );
		return false;
	}
}

void http::ClientEventProcessor::executeRequest( Client &client ) {

	ServerConfig &serverInfo = _server._serverInfo;

	std::cout << "In executeRequest\n";
	HttpRouter::routeRequest( client, serverInfo );

	client.clearBuffers();
}

bool http::ClientEventProcessor::sendResponse( pollfd &pfd, Client &client ) {
	SocketFD clientFd = client.getFd();
	const int MAX_SENDS_PER_EVENT = 3;

	// Build response if write buffer is empty
	if ( client.getWriteBuffer().empty() ) {
		client.appendToWriteBuffer( client.getResponse().buildResponseString( client.getRequest() ) );
	}

	std::string &writeBuffer = client.getWriteBuffer();

	if ( writeBuffer.empty() )
		return 0;

	int sendCount = 0;

	// Send up to MAX_SENDS_PER_EVENT times per poll event
	while ( sendCount < MAX_SENDS_PER_EVENT && !writeBuffer.empty() ) {
		ssize_t bytesSent = send( clientFd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL );

		if ( bytesSent < 0 ) {
			if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
				// Socket buffer full, will continue later
				pfd.events |= POLLOUT;
				return 1; // Keep connection alive, continue sending later
			}

			if ( errno == EPIPE ) {
				Logs::log( ERROR, "Client disconnected before response" );
			} else {
				Logs::log( ERROR, "Error sending response to client" );
			}
			return 1; // Close connection
		}

		if ( bytesSent == 0 ) {
			// Should not happen with send(), but handle gracefully
			break;
		}

		writeBuffer.erase( 0, bytesSent );
		sendCount++;
	}

	// Check if all data was sent
	if ( writeBuffer.empty() ) {
		std::string msg( "Server Response sent to client " );
		msg += to_str( clientFd );
		if ( DEBUG ) {
			msg += " ";                      //! For Debug
			msg += client.getRequest().path; //! For Debug
		}
		Logs::log( INFO, msg );

		if ( client.getRequest().shouldCloseConnection() ) {
			_server._clientManager.resetClientState( clientFd );
			return 1; // Close connection
		}

		_server._clientManager.resetClientState( clientFd );
		pfd.events = POLLIN; // Reset to read for next request
		return 0;
	}

	// Still have data to send, keep POLLOUT active
	pfd.events |= POLLOUT;
	return 1; // Continue sending in next poll event
}

void http::ClientEventProcessor::processClientEvents() {

	for ( size_t i = 1; i < _server._fds.size(); ++i ) { // Main loop
		Client *client = _server._clientManager.getClient( _server._fds[ i ].fd );
		if ( !client ) {
			_server.closeClientConnection( i );
			continue;
		}

		if ( _server._fds[ i ].revents & POLLIN ) { //
			processRead( _server._fds[ i ], *client );
		}

		if ( _server._fds[ i ].revents & POLLOUT ) { // Read child process CGI, creates response and sent to the client
			processWrite( _server._fds[ i ], *client );
		}
	}
}