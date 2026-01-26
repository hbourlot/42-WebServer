#include "httpTcpServer/Cgi.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <vector>

http::Cgi::Cgi( const http::Request& request, const std::string& scriptPath, const ServerConfig& serverInfo,
                Client* client )
    : _status(), _clientFD(), _request( request ), _response( Response( request ) ), _serverInfo( serverInfo ),
      _clientAddress(), _bytesReceived(), _bodyBytesWritten( 0 ), _body(), _client( client ), _envp(), _argv(),
      _envStrings() {

	_filePath = scriptPath;

	buildEnvStrings();
}

http::Cgi::~Cgi() {
	// Close all pipe fds
	if ( _inputPipe[ 0 ] >= 0 )
		close( _inputPipe[ 0 ] );
	if ( _inputPipe[ 1 ] >= 0 )
		close( _inputPipe[ 1 ] );
	if ( _outputPipe[ 0 ] >= 0 )
		close( _outputPipe[ 0 ] );
	if ( _outputPipe[ 1 ] >= 0 )
		close( _outputPipe[ 1 ] );

	remove( _bodyFileName.c_str() );
}

std::string http::Cgi::getFilePath() const {
	return this->_filePath;
}

http::Request http::Cgi::getRequest() const {
	return _request;
}

http::Response http::Cgi::getResponse() const {
	return _response;
}

int http::Cgi::getPollFd() const {
	return _outputPipe[ 0 ];
}

std::string http::Cgi::getBody() const {
	return _body;
}

pid_t http::Cgi::getPid() const {
	return _pid;
}

const int* http::Cgi::getInputPipe() const {
	return _inputPipe;
}

int http::Cgi::getStatus() const {
	return _status;
}
int& http::Cgi::getStatus() {
	return _status;
}

const int* http::Cgi::getOutputPipe() const {
	return _outputPipe;
}

Client* http::Cgi::getClient() const {
	return _client;
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

void http::Cgi::doDupOneWay() {
	// One-way: CGI writes to stdout only (no stdin from parent)
	dup2( _outputPipe[ 1 ], STDOUT_FILENO );

	// Close all pipe fds
	close( _inputPipe[ 0 ] );
	close( _inputPipe[ 1 ] );
	close( _outputPipe[ 0 ] );
	close( _outputPipe[ 1 ] );
}

void http::Cgi::doDupTwoWay() {
	dup2( _inputPipe[ 0 ], STDIN_FILENO );
	dup2( _outputPipe[ 1 ], STDOUT_FILENO );

	// Close all pipe fds - we now use stdin/stdout
	close( _inputPipe[ 0 ] );
	close( _inputPipe[ 1 ] );
	close( _outputPipe[ 0 ] );
	close( _outputPipe[ 1 ] );
}

void http::Cgi::closeForOneWay() {
	// One-way: parent only reads from CGI output
	close( _inputPipe[ 0 ] );  // Not using stdin pipe
	close( _inputPipe[ 1 ] );  // Not using stdin pipe
	close( _outputPipe[ 1 ] ); // Child writes to stdout
	                           // Keep _outputPipe[0] to read from CGI
}

void http::Cgi::closeForTwoWay() {
	// Two-way: parent writes to CGI stdin and reads from stdout
	close( _inputPipe[ 0 ] );  // Child reads from stdin
	close( _outputPipe[ 1 ] ); // Child writes to stdout
	                           // Keep _inputPipe[1] to write to CGI
	                           // Keep _outputPipe[0] to read from CGI
}

void http::Cgi::killProcess() {
	if ( _pid > 0 ) {
		// Check if process is still running
		int status;
		pid_t result = waitpid( _pid, &status, WNOHANG );

		if ( result == 0 ) { // Process still running - kill it
			kill( _pid, SIGKILL );
			waitpid( _pid, &status, 0 ); // Reap zombie
		}
	}
}

void http::Cgi::writeToCgi() {
	if ( _bodyBytesWritten >= _request.body.size() )
		return;

	size_t remaining = _request.body.size() - _bodyBytesWritten;
	ssize_t written = write( _inputPipe[ 1 ], _request.body.c_str() + _bodyBytesWritten, remaining );

	if ( written > 0 ) {
		_bodyBytesWritten += written;
	}
}

size_t http::Cgi::getBodyBytesWritten() const {
	return _bodyBytesWritten;
}
