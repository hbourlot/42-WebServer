#include "Client/ClientEventProcessor.hpp"
#include "Logs/Logs.hpp"
#include "utils.hpp"
#include <unistd.h>

void http::ClientEventProcessor::registerCgiPipes( const int inputPipe[ 2 ], const int outputPipe[ 2 ], pid_t pid ) {
	CgiPipeFds cgiPipes;
	cgiPipes.inputPipe[ 0 ] = inputPipe[ 0 ];
	cgiPipes.inputPipe[ 1 ] = inputPipe[ 1 ];
	cgiPipes.outputPipe[ 0 ] = outputPipe[ 0 ];
	cgiPipes.outputPipe[ 1 ] = outputPipe[ 1 ];
	cgiPipes.pid = pid;

	_server._cgiPipes.push_back( cgiPipes );

	std::string msg( "Registered CGI pipes for PID " );
	msg += to_str( pid );
	Logs::log( INFO, msg );
}

void http::ClientEventProcessor::closeCgiPipes( pid_t pid ) { // ! TODO: [] Could be a map to be faster
	for ( size_t i = 0; i < _server._cgiPipes.size(); ++i ) {
		if ( _server._cgiPipes[ i ].pid == pid ) {
			// Close all 4 pipe fds
			if ( _server._cgiPipes[ i ].inputPipe[ 0 ] >= 0 )
				close( _server._cgiPipes[ i ].inputPipe[ 0 ] );
			if ( _server._cgiPipes[ i ].inputPipe[ 1 ] >= 0 )
				close( _server._cgiPipes[ i ].inputPipe[ 1 ] );
			if ( _server._cgiPipes[ i ].outputPipe[ 0 ] >= 0 )
				close( _server._cgiPipes[ i ].outputPipe[ 0 ] );
			if ( _server._cgiPipes[ i ].outputPipe[ 1 ] >= 0 )
				close( _server._cgiPipes[ i ].outputPipe[ 1 ] );

			_server._cgiPipes.erase( _server._cgiPipes.begin() + i );

			std::string msg( "Closed CGI pipes for PID " );
			msg += to_str( pid );
			Logs::log( INFO, msg );
			return;
		}
	}
}

void http::ClientEventProcessor::registerCgiForClient( Client &client, int cgiOutputFd ) {
	_server._cgiFdToClient[ cgiOutputFd ] = &client;
}

void http::ClientEventProcessor::cleanupCgiForClient( Client &client ) {
	if ( client.getCgiPid() != -1 ) {
		closeCgiPipes( client.getCgiPid() );
	}
	if ( client.getCgiOutputFd() != -1 ) {
		_server._cgiFdToClient.erase( client.getCgiOutputFd() );
		client.setCgiOutputFd( -1 );
	}
	client.setCgiPid( -1 );
}

void http::ClientEventProcessor::processCgiOutput( Client &client, pollfd &pfd ) {

	// 1 Read available data from CGI pipe (in non-blocking state)
	char buffer[ BUFFER_SIZE ];
	ssize_t bytesRead = read( client.getCgiOutputFd(), buffer, BUFFER_SIZE );

	if ( bytesRead > 0 ) {

		// ! Appending to client's CGI response body ?? Jorge
		// client.getReadBuffer().append(buffer, bytesRead); //! Verify if it's empty after request
		client.getWriteBuffer().append( buffer, bytesRead );
	} else if ( bytesRead == 0 ) {

		// Prob finished writing - Need to check from here
		std::cout << "CGi finished writing, PID: " << client.getCgiPid() << std::endl;

		int status;
		pid_t result = waitpid( client.getCgiPid(), &status, WNOHANG );

		if ( result > 0 ) {
			// CGI process has finished

			// Prob these two cases needs to be in processor.writeProcess()
			if ( WIFEXITED( status ) && WEXITSTATUS( status ) == 0 ) {
				// Success Exit - build response form collected output
				std::string cgiOutput = client.getWriteBuffer();
				client.getResponse().buildCgiResponse( HTTP_OK, cgiOutput, _server._serverInfo );

				// Mark Client ready to write response back
				pfd.events = POLLOUT;
			} else {
				// Cgi failed
				client.getResponse().buildErrorResponse( HTTP_SERVER_ERR, _server._serverInfo );
			} // TODO: [] Also needs to check result == 0 which means it's still working and could send response as
			  // TODO: Chunks

			// CleanupCgi resources
			cleanupCgiForClient( client );
			client.setState( CGI_COMPLETED );
		}
	} else if ( bytesRead == -1 ) {
		if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
			// No data available right now, will be called again on next POLLIN
			return;
		} else {
			// Read Error
			// Prob needs to build response at processor.writeProcess()
			std::cerr << "Error reading from CGI pipe: " << strerror( errno ) << std::endl;
			client.getResponse().buildErrorResponse( HTTP_SERVER_ERR, _server._serverInfo );
			cleanupCgiForClient( client );
			client.setState( CGI_COMPLETED );
		}
	}
}