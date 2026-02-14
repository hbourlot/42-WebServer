#include "Client/ClientEventProcessor.hpp"
#include "Logs/Logs.hpp"
#include "utils.hpp"
#include <unistd.h>

void http::ClientEventProcessor::registerCgi( http::Cgi* cgi ) {
	int outputFd = cgi->getOutputPipe()[ 0 ];

	_cgi_by_fd[ outputFd ] = cgi; // Add CGI to map

	// Handling Output Pipe (Reading from CGI)
	pollfd pfd;

	pfd.fd = cgi->getOutputPipe()[ 0 ];
	pfd.events = POLLIN;
	pfd.revents = 0;
	_allSockets.push_back( pfd );

	std::string msg( "Registered CGI for PID " );
	msg += ft_to_string( cgi->getPid() );
	msg += " with output fd ";
	msg += ft_to_string( outputFd );
	Logs::log( LOGS_INFO, msg );
}

void http::ClientEventProcessor::cleanupCgi( http::Cgi* cgi ) {
	int outputFd = cgi->getOutputPipe()[ 0 ];
	Client* client = cgi->getClient();

	cgi->killProcess(); // Kill CGI process if still running

	// Remove CGI pipe fd from poll array BEFORE deleting Cgi (which closes pipes)
	for ( size_t i = 0; i < _cgi_by_fd.size(); ++i ) {
		if ( _allSockets[ i ].fd == outputFd ) {
			_allSockets.erase( _allSockets.begin() + i );
			break;
		}
	}

	// Remove from map
	_cgi_by_fd.erase( outputFd );

	// Reset client CGI state
	if ( client ) {
		client->setCgiPid( -1 );
		client->setCgiOutputFd( -1 );
	}

	// Delete Cgi object (destructor closes pipes)
	delete cgi;

	std::string msg( "Cleaned up CGI with output fd " );
	msg += ft_to_string( outputFd );
	Logs::log( LOGS_INFO, msg );
}

bool http::ClientEventProcessor::hasCgiFinished( Cgi* cgi ) const {

	pid_t result = waitpid( cgi->getPid(), &cgi->getStatus(), WNOHANG );
	if ( result > 0 )
		return true;
	return false;
};

bool http::ClientEventProcessor::hasCgiSuccessfullyFinished( Cgi* cgi ) const {
	int& status = cgi->getStatus();

	if ( WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
		return true;
	return false;
}
