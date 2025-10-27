#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

bool http::TcpServer::handleCgiResponse( pollfd &socket ) {
	Client *client = _clientManager.getClient( socket.fd );

	std::cout << "[DEBUG] FD: " << socket.fd << " | revents: " << socket.revents << " | events: " << socket.events
	          << std::endl;
	if ( ( socket.revents & POLLIN ) && _cgiFdMap.count( socket.fd ) ) {
		std::cout << "FD ON HANDLE => " << socket.fd << std::endl;
		Cgi *cgi = _cgiFdMap[ socket.fd ];
		cgi->readCgiOutput();

		client->getResponse() = ResponseBuilder::buildResponse( HTTP_OK, cgi->getBody() );

		std::cout << cgi->getBody();
		sendResponse( socket );
		_cgiFdMap.erase( socket.fd );
		return true;
	}
	return false;
}

void http::TcpServer::closeClientConnection( size_t index ) {
	SocketFD fd = _fds[ index ].fd;

	// std::cout << "Closing client FD => " << fd << std::endl;
	std::string msg( "Closing FD => " );
	msg += to_str( fd );

	Logs::log( ERROR, msg );

	close( fd );
	_socketAddressMap.erase( fd );
	_clientManager.removeClient( fd );
	_fds.erase( _fds.begin() + index );
}

bool http::TcpServer::handleReadStatus( READ_STATUS status, Client *client, size_t index ) {

	switch ( status ) {
	case READ_SUCCESS:
		return true;

	case READ_CLIENT_CLOSED:
	case READ_CLIENT_NOT_FOUND:
		closeClientConnection( index );
		return false;
	case READ_EMPTY:
		client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_BAD_REQ );
		_fds[ index ].events |= POLLOUT;
		return true; // ! Attention which return should be

	case READ_INCOMPLETE:
		return false; // Need more data, stop processing

	default:
		return false;
	}
}

PARSE_STATUS http::TcpServer::parseClientRequest( Client *client ) {

	return parseRequest( client, _serverInfo );
}

bool http::TcpServer::handleParseStatus( PARSE_STATUS status, Client *client, size_t index ) {

	switch ( status ) {
	case PARSE_OK:
		return true;

	case PARSE_TOO_LARGE:
		client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_PAYLOAD );
		_fds[ index ].events |= POLLOUT;
		return false;

	case PARSE_INCOMPLETE:
		return false; // Need more data

	default:
		client->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_SERVER_ERR );
		_fds[ index ].events |= POLLOUT;
		return false;
	}
}

void http::TcpServer::routeClientRequest( Client *client, size_t index ) {

	HttpRouter::handleMethods( *client, _serverInfo );

	client->clearBuffers();
	_fds[ index ].events |= POLLOUT;
}

bool http::TcpServer::handleRouteValidation( Client *client, size_t index ) {
	httpResponse &response = client->getResponse();

	VALIDATION_STATUS validationStatus = HttpRouter::validateRequest( *client, _serverInfo );

	switch ( validationStatus ) {
	case VALID_OK:
		return true; // Continue to routing

	case VALID_NOT_FOUND:
		response =
		    ResponseBuilder::buildFileResponse( HTTP_NOT_FOUND, _serverInfo.errorPage.at( 404 ), _serverInfo, true );
		_fds[ index ].events |= POLLOUT;
		return false;

	case VALID_REDIRECT_REQUIRED:
		response = ResponseBuilder::buildRedirect( HTTP_MOVED, client->getRequest().urlMatchedLocation->redirection );
		_fds[ index ].events |= POLLOUT;
		return false;

	case VALID_METHOD_NOT_ALLOWED:
		response = ResponseBuilder::buildFileResponse( HTTP_FORBID_METHOD, DFL_405, _serverInfo, true );
		_fds[ index ].events |= POLLOUT;
		return false;

	default:
		response = ResponseBuilder::buildErrorResponse( HTTP_SERVER_ERR );
		_fds[ index ].events |= POLLOUT;
		return false;
	}
}

void http::TcpServer::handleClientRead( size_t index ) {

	// Step 1: check client conenction
	Client *client = _clientManager.getClient( _fds[ index ].fd );
	if ( !client ) {
		closeClientConnection( index ); // TODO: Maybe some output message?
		return;
	}
	// Step 2: Read from socket
	READ_STATUS readStatus = readRequest( index );

	if ( !handleReadStatus( readStatus, client, index ) ) {

		return; // Error handled, move on
	}

	// Step 3: Parse Request
	PARSE_STATUS parseStatus = parseClientRequest( client );

	if ( !handleParseStatus( parseStatus, client, index ) ) {
		return;
	}

	// Step 4: Validate Route
	if ( !handleRouteValidation( client, index ) )
		return;

	// Step 5: Route and handle request
	routeClientRequest( client, index );
}

void http::TcpServer::processClientEvents() {

	READ_STATUS requestStatus;
	SocketFD fd;
	bool shouldCloseSend;
	bool shouldCloseRead;
	sockaddr_in *currentAddress;

	for ( size_t idx = 1; idx < _fds.size(); ++idx ) {
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = _fds[ idx ].fd;

		if ( _fds[ idx ].revents & POLLIN ) {
			// shouldCloseRead = readRequest( idx );

			// requestStatus = readRequest( idx );
			// if ( requestStatus == READ_ERROR ) {
			// 	// * Should close the specific client
			// } else if ( requestStatus == READ_EMPTY ) {
			// 	// * Build response HTTP_BAD_REQ
			// 	_clientManager.getClient( fd )->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_BAD_REQ );
			// 	_fds[ idx ].events |= POLLOUT;
			// } else {
			// 	// * Parse Request
			// 	PARSE_STATUS parseStatus = parseRequest( _clientManager.getClient( fd ), _serverInfo );
			// 	if ( parseStatus == PARSE_INCOMPLETE ) {
			// 		// * Move further
			// 		continue;
			// 	} else if ( parseStatus == PARSE_TOO_LARGE ) {
			// 		_clientManager.getClient( fd )->getResponse() = ResponseBuilder::buildErrorResponse( HTTP_PAYLOAD );
			// 		_fds[ idx ].events |= POLLOUT;
			// 	} else {
			// 		// * PARSE_OK => validate URL
			// 		httpResponse &response = _clientManager.getClient( fd )->getResponse();
			// 		VALIDATION_STATUS validationStatus =
			// 		    HttpRouter::validateRequest( *_clientManager.getClient( fd ), _serverInfo );
			// 		if ( validationStatus == VALID_NOT_FOUND ) {
			// 			response = ResponseBuilder::buildFileResponse( HTTP_NOT_FOUND, _serverInfo.errorPage.at( 404 ),
			// 			                                               _serverInfo, true );
			// 			_fds[ idx ].events |= POLLOUT;
			// 		} else if ( validationStatus == VALID_REDIRECT_REQUIRED ) {
			// 			response = ResponseBuilder::buildRedirect(
			// 			    HTTP_MOVED, _clientManager.getClient( fd )->getRequest().urlMatchedLocation->redirection );
			// 			_fds[ idx ].events |= POLLOUT;
			// 		} else if ( validationStatus == VALID_ERROR ) {
			// 			response = ResponseBuilder::buildFileResponse( HTTP_FORBID_METHOD, DFL_405, _serverInfo, true );
			// 			_fds[ idx ].events |= POLLOUT;
			// 		} else {
			// 			HttpRouter::handleMethods( *_clientManager.getClient( fd ), _serverInfo );
			// 			_fds[ idx ].events |= POLLOUT;
			// 		}
			// 	}
			// }

			handleClientRead( idx );
		}

		if ( _fds[ idx ].revents & POLLOUT ) {
			int sendResult = sendResponse( _fds[ idx ] );
			if ( sendResult == 1 )
				shouldCloseSend = true;
			else if ( sendResult == 2 )
				_fds[ idx ].events |= POLLOUT;
			else
				_fds[ idx ].events &= ~POLLOUT;
		}
		if ( shouldCloseRead || shouldCloseSend ) {
			closeClientConnection( idx );
			--idx;
			continue;
		}
		// _fds[idx].events |= POLLIN;
	}
}