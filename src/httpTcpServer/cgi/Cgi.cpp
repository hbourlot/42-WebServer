#include "httpTcpServer/Cgi.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <vector>

std::string http::Cgi::getFilePath() const {
	return this->_filePath;
}

httpRequest http::Cgi::getRequest() const {
	return _request;
}

HttpResponse http::Cgi::getResponse() const {
	return _response;
}

int http::Cgi::getPollFd() const {
	return _outputPipe[ 0 ];
}

http::Cgi::CgiStatus http::Cgi::getStatus() const {
	return _status;
}

std::string http::Cgi::getBody() const {
	return _body;
}

void http::Cgi::registerPollFd( std::vector< pollfd > &fds ) const {
	pollfd pfd;

	pfd.fd = _outputPipe[ 0 ];
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back( pfd );
}

void http::Cgi::markAsRunning() {
	this->_status = CGI_RUNNING;
}

http::Cgi::Cgi( const httpRequest &request, std::string &filePath, const sockaddr_in &clientAddress,
                const ServerConfig &serverInfo )
    : _status( CGI_NOT_STARTED ), _clientFD(), _request( request ), _response( HttpResponse( request ) ),
      _serverInfo( serverInfo ), _filePath( filePath ), _clientAddress( clientAddress ), _bytesReceived(), _body(),
      _envp(), _argv(), _envStrings() {

	// Cgi::createValidCgiExtensions();

	// execve
	buildEnvStrings();
}

http::Cgi::~Cgi() {
	close( _inputPipe[ 0 ] );
	close( _inputPipe[ 1 ] );
	close( _outputPipe[ 0 ] );
	close( _outputPipe[ 1 ] );
}

void http::Cgi::updateStatus() {

	int status;
	std::cout << "CHECKING RESULT -------------------\n";
	pid_t result = waitpid( _pid, &status, WNOHANG );
	std::cout << "Result => " << result << "-------------------\n";
	if ( result > 0 ) { // Child Process has finished
		if ( WIFEXITED( status ) ) {
			int exitCode = WEXITSTATUS( status );
			if ( exitCode == 0 ) { // Finished successfully
				std::cout << "SUCCESS\n";
				_status = CGI_FINISHED;
			} else { // Finished with error
				_status = CGI_ERROR;
			}
		} else if ( WIFSIGNALED( status ) ) { // Process was killed by signal
			_status = CGI_ERROR;
		}
	} else if ( result == 0 ) { // Still running
		if ( _status == CGI_NOT_STARTED ) {
			_status = CGI_RUNNING;
		}
	} else { // Error in waitpid
		_status = CGI_ERROR;
	}
}

bool http::Cgi::isCgiFinished() {
	this->updateStatus();
	return _status == CGI_FINISHED ? true : false;
}

bool http::Cgi::hasDataToRead() {

	char testByte;

	ssize_t result = recv( _outputPipe[ 0 ], &testByte, 1, MSG_PEEK );
	//  | MSG_DONTWAIT

	if ( result > 0 )
		return true; // Data available

	if ( result == 0 )
		return false; // EOF - pipe closed

	if ( errno == EAGAIN || errno == EWOULDBLOCK )
		return false; // No data available

	return false; // Any other error
}

bool http::Cgi::processCgiOut() {

	// // this->readCgiOutput();
	_response.buildResponse( HTTP_OK, _body );
	return true;

	this->updateStatus();
	CgiStatus status = _status;
	const std::string bodyError =
	    "<html><body><h1>500 Internal Server Error</h1><p>Error monitoring CGI process.</p></body></html>";

	if ( status == CGI_FINISHED ) {
		this->readCgiOutput();

		if ( !_body.empty() ) {
			std::cerr << "HERE\n";
			_response.buildResponse( HTTP_OK, _body );
		} else {
			std::cerr << "HERE1\n";
			_response.buildResponse( HTTP_SERVER_ERR, bodyError );
		}
		return true;

	} else if ( status == CGI_RUNNING ) {
		if ( this->hasDataToRead() ) {
			this->readCgiOutput();
			// send as chunk
			return false;
		}
	} else if ( status == CGI_ERROR ) {
		std::cout << "CGI_ERROR\n";
		_response.buildResponse( HTTP_SERVER_ERR, bodyError );
	};

	return true;
}