#include "Client/ClientEventProcessor.hpp"
#include "Logs/Logs.hpp"
#include "utils.hpp"
#include <unistd.h>

void http::ClientEventProcessor::registerCgi( http::Cgi *cgi ) {
	int outputFd = cgi->getOutputPipe()[ 0 ];

	_server._cgiByFd[ outputFd ] = cgi; // Add CGI to map

	std::string msg( "Registered CGI for PID " );
	msg += to_str( cgi->getPid() );
	msg += " with output fd ";
	msg += to_str( outputFd );
	Logs::log( LOGS_INFO, msg );
}

void http::ClientEventProcessor::cleanupCgi( http::Cgi *cgi ) {
	int outputFd = cgi->getOutputPipe()[ 0 ];
	Client *client = cgi->getClient();

	cgi->killProcess(); // Kill CGI process if still running

	// Remove CGI pipe fd from poll array BEFORE deleting Cgi (which closes pipes)
	for ( size_t i = 0; i < _server._fds.size(); ++i ) {
		if ( _server._fds[ i ].fd == outputFd ) {
			_server._fds.erase( _server._fds.begin() + i );
			break;
		}
	}

	// Remove from map
	_server._cgiByFd.erase( outputFd );

	// Reset client CGI state
	if ( client ) {
		client->setCgiPid( -1 );
		client->setCgiOutputFd( -1 );
	}

	// Delete Cgi object (destructor closes pipes)
	delete cgi;

	std::string msg( "Cleaned up CGI with output fd " );
	msg += to_str( outputFd );
	Logs::log( LOGS_INFO, msg );
}

bool http::ClientEventProcessor::hasCgiFinished( Cgi *cgi ) const {

	pid_t result = waitpid( cgi->getPid(), &cgi->getStatus(), WNOHANG );
	if ( result > 0 )
		return true;
	return false;
};

bool http::ClientEventProcessor::hasCgiSuccessfullyFinished( Cgi *cgi ) const {
	int &status = cgi->getStatus();

	if ( WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
		return true;
	return false;
}

void http::ClientEventProcessor::processCgiOutput( http::Cgi *cgi, pollfd &pfd ) {

	Client *client = cgi->getClient();
	if ( !client ) {
		std::cerr << "Error: CGI has no associated client\n";
		cleanupCgi( cgi );
		return;
	}

	// Read all available data from CGI pipe (non-blocking)
	char buffer[ BUFFER_SIZE ];
	std::string cgiOutput;
	int readCount = 0;

	while ( readCount < MAX_READS_PER_EVENT ) {
		ssize_t bytesRead = read( pfd.fd, buffer, BUFFER_SIZE );

		if ( bytesRead > 0 ) {
			cgiOutput.append( buffer, bytesRead );
			readCount++;
		} else if ( bytesRead == 0 ) {
			// EOF - finished reading
			break;
		} else if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
			// No more data available right now
			break;
		} else {
			// Read error
			// std::cerr << "Error reading from CGI pipe: " << strerror( errno ) << std::endl;
			client->getResponse().buildErrorResponse( HTTP_SERVER_ERR, _server._serverInfo );
			pfd.fd = -1;
			cleanupCgi( cgi );
			// client->setState( CGI_COMPLETED );
			return;
		}
	}

	// Build response based on CGI exit status
	if ( hasCgiSuccessfullyFinished( cgi ) ) {

		if ( cgiOutput.empty() ) {
			// CGI produced no output
			client->getResponse().buildErrorResponse( HTTP_SERVER_ERR, _server._serverInfo );
		} else {
			// Success - build response from CGI output
			client->getResponse().buildCgiResponse( HTTP_OK, cgiOutput, _server._serverInfo );
		}
	} else {
		// CGI failed (non-zero exit status)
		client->getResponse().buildErrorResponse( HTTP_SERVER_ERR, _server._serverInfo );
	}

	// Mark pipe fd for removal and cleanup
	pfd.fd = -1;
	cleanupCgi( cgi );
	client->setState( CGI_COMPLETED );
}