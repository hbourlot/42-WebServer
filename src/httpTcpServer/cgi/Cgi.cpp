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

httpRequest http::Cgi::getCgiRequest() const {
	return _request;
}

httpResponse http::Cgi::getCgiResponse() const {
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

http::Cgi::Cgi( httpRequest &request, std::string &filePath, const sockaddr_in &clientAddress,
                const ServerConfig &serverInfo )
    : _request( request ), _filePath( filePath ), _clientAddress( clientAddress ), _serverInfo( serverInfo ), _envp(),
      _argv(), _envStrings(), _body(), _inputPipe(), _outputPipe(), _clientFD() {

	// Cgi::createValidCgiExtensions();

	// execve
	buildEnvStrings();
	_status = CGI_NOT_STARTED;
}

http::Cgi::~Cgi() {
	close( _inputPipe[ 0 ] );
	close( _inputPipe[ 1 ] );
	close( _outputPipe[ 0 ] );
	close( _outputPipe[ 1 ] );
}

void http::Cgi::updateStatus() {
	int status;
	pid_t result = waitpid( _pid, &status, WNOHANG );

	if ( result > 0 ) { // Child Process has finished
		if ( WIFEXITED( status ) ) {
			int exitCode = WEXITSTATUS( status );
			if ( exitCode == 0 ) { // Finished successfully
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

	this->updateStatus();
	CgiStatus status = _status;

	if ( status == CGI_FINISHED ) {
		this->readCgiOutput();

		if ( !_body.empty() ) {
			_response.body = _body;
			_response.statusCode = "200";
			_response.statusMsg = "OK";
		} else {
			_response.body =
			    "<html><body><h1>500 Internal Server Error</h1><p>Error monitoring CGI process.</p></body></html>";
			_response.statusCode = "500";
			_response.statusMsg = "Internal Server Error";
		}
		return true;

	} else if ( status == CGI_RUNNING ) {
		if ( this->hasDataToRead() ) {
			this->readCgiOutput();
			// send as chunk
			return false;
		}
	} else if ( status == CGI_ERROR ) {
		_response.body =
		    "<html><body><h1>500 Internal Server Error</h1><p>Error monitoring CGI process.</p></body></html>";
		_response.statusCode = "500";
		_response.statusMsg = "Internal Server Error";
	};
	return true;
}

// void Cgi::processCgiOut() {
// 	// Check if CGI process finished and handle the result
// 	int status;
// 	pid_t result = waitpid( _pid, &status, WNOHANG );

// 	if ( result > 0 ) {
// 		// Child process has finished
// 		if ( WIFEXITED( status ) && WEXITSTATUS( status ) == 0 ) {
// 			// CGI script executed successfully
// 			if ( !_body.empty() ) {
// 				// Set successful response with CGI output
// 				_response.body = _body;
// 				_response.statusCode = HTTP_OK.code;
// 				_response.statusMsg = HTTP_OK.message;
// 				_status = CGI_FINISHED;
// 			} else {
// 				// CGI finished but no output - send error page
// 				_response.body = "<html><body><h1>500 Internal Server Error</h1><p>CGI script produced no "
// 				                 "output.</p></body></html>";
// 				_response.statusCode = HTTP_SERVER_ERR.code;
// 				_response.statusMsg = HTTP_SERVER_ERR.message;
// 				_status = CGI_ERROR;
// 			}
// 		} else {
// 			// CGI script failed or exited with error
// 			_response.body =
// 			    "<html><body><h1>500 Internal Server Error</h1><p>CGI script execution failed.</p></body></html>";
// 			_response.statusCode = HTTP_SERVER_ERR.code;
// 			_response.statusMsg = HTTP_SERVER_ERR.message;
// 			_status = CGI_ERROR;
// 		}

// 		// Close pipes and clean up
// 		close( _outputPipe[ 0 ] );
// 		close( _inputPipe[ 1 ] );

// 	} else if ( result == 0 ) {
// 		// Child process still running, keep current status
// 		if ( _status == CGI_NOT_STARTED ) {
// 			_status = CGI_RUNNING;
// 		}
// 	} else {
// 		// Error in waitpid
// 		_response.body =
// 		    "<html><body><h1>500 Internal Server Error</h1><p>Error monitoring CGI process.</p></body></html>";
// 		_response.statusCode = HTTP_SERVER_ERR.code;
// 		_response.statusMsg = HTTP_SERVER_ERR.message;
// 		_status = CGI_ERROR;
// 	}
// }