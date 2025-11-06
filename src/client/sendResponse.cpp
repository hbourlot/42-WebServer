#include "Client/ClientEventProcessor.hpp"


bool http::ClientEventProcessor::sendResponse( pollfd &pfd, Client &client ) {
	SocketFD clientFd = client.getFd();
	const int MAX_SENDS_PER_EVENT = 3;

	// Build response if write buffer is empty
	if ( client.getWriteBuffer().empty() ) {
		client.appendToWriteBuffer( client.getResponse().buildResponseString( ) );
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