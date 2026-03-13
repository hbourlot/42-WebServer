#include "httpTcpServer/Cgi.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

http::Cgi::Cgi( const http::Request &request, const ServerConfig &serverInfo, Client *client )
	: _request( request ), _serverInfo( serverInfo ), _client( client ), _pid( -1 ), _status( 0 ), _stdinFd( -1 ),
	  _clientAddress( client && client->getServer().getSocketAddressRef().count( client->getFd() )
						  ? client->getServer().getSocketAddressRef()[client->getFd()]
						  : sockaddr_in() ),
	  _envp(), _envStrings(), _state( RESET ), _hasFinished( false ) {

	_filePath.clear();
	if ( request.getMatchLocation() && !request.getMatchLocation()->cgi_pass.empty() ) {
		_filePath = request.getMatchLocation()->cgi_pass;
	} else {
		_filePath = request.getFullPath();
	}
	_outputPipe[0] = -1;
	_outputPipe[1] = -1;

	buildEnvStrings();
}

http::Cgi::~Cgi() {
	// Close CGI fds
	if ( _stdinFd >= 0 )
		close( _stdinFd );
	if ( _outputPipe[0] >= 0 )
		close( _outputPipe[0] );
	if ( _outputPipe[1] >= 0 )
		close( _outputPipe[1] );

	remove( _bodyFileName.c_str() );
}

std::string http::Cgi::getFilePath() const {
	return this->_filePath;
}

pid_t http::Cgi::getPid() const {
	return _pid;
}

int http::Cgi::getStatus() const {
	return _status;
}
int &http::Cgi::getStatus() {
	return _status;
}

const int *http::Cgi::getOutputPipe() const {
	return _outputPipe;
}

Client *http::Cgi::getClient() const {
	return _client;
}

int http::Cgi::getOutputPipeFd() const {
	return _outputPipe[0];
}

void http::Cgi::dupCgiFds() {
	if ( dup2( _stdinFd, STDIN_FILENO ) < 0 )
		std::cerr << "ERROR ON DUP2 STDIN\n" << std::endl;
	if ( dup2( _outputPipe[1], STDOUT_FILENO ) < 0 )
		std::cerr << "ERROR ON DUP2 STDOUT\n" << std::endl;

	// Close original fds in child after dup
	close( _stdinFd );
	close( _outputPipe[0] );
	close( _outputPipe[1] );
	_stdinFd = -1;
	_outputPipe[0] = -1;
	_outputPipe[1] = -1;
}

void http::Cgi::killProcess() {
	if ( _pid > 0 ) {
		// Check if process is still running
		int status;
		pid_t result = waitpid( _pid, &status, WNOHANG );

		if ( result == 0 ) { // Process still running - kill it
			kill( _pid, SIGKILL );
			waitpid( _pid, &status, 0 );
		}
	}
}

int http::Cgi::validateFilePermission() {
	struct stat st;
	if ( stat( this->_filePath.c_str(), &st ) != 0 ) {
		Logs::log( LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() + "': File doesn't exist" );
		return ( -1 );
	}
	if ( !( st.st_mode & X_OK ) ) {
		Logs::log( LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() +
								   "': File doesn't have executable permision" );
		return ( -1 );
	}
	return ( 0 );
}

int http::Cgi::prepareCgiInputFile() {
	std::string templatePath = joinPath( _serverInfo.temp_path, "_" + _client->getSessionID() + "_content_XXXXXX" );
	std::vector< char > templateBuffer( templatePath.begin(), templatePath.end() );
	templateBuffer.push_back( '\0' );

	if ( _stdinFd >= 0 ) {
		close( _stdinFd );
		_stdinFd = -1;
	}

	_stdinFd = mkstemp( templateBuffer.data() );
	if ( _stdinFd < 0 ) {
		Logs::log( LOGS_ERROR, "Failed to create temp file for CGI input: " + std::string( strerror( errno ) ) );
		return -1;
	}
	_bodyFileName = templateBuffer.data();

	if ( !_request.writeBodyToFd( _stdinFd ) ) {
		Logs::log( LOGS_ERROR, "Failed to write to file: " + std::string( strerror( errno ) ) );
		close( _stdinFd );
		_stdinFd = -1;
		return -1;
	}

	// Ensure all data is written to disk before forking
	if ( fsync( _stdinFd ) == -1 ) {
		Logs::log( LOGS_ERROR, "Failed to fsync file: " + std::string( strerror( errno ) ) );
		close( _stdinFd );
		_stdinFd = -1;
		return -1;
	}

	// After writing, seek back to the beginning of the file for reading
	if ( lseek( _stdinFd, 0, SEEK_SET ) == -1 ) {
		Logs::log( LOGS_ERROR, "Failed to seek file for reading: " + std::string( strerror( errno ) ) );
		close( _stdinFd );
		_stdinFd = -1;
		return -1;
	}

	return 0;
}

bool http::Cgi::executeCgi() {
	if ( validateFilePermission() )
		return ( -1 );

	if ( prepareCgiInputFile() != 0 ) {
		return -1;
	}

	if ( pipe( this->_outputPipe ) < 0 ) {
		Logs::log( LOGS_ERROR, "Pipe creating failed" );
		if ( _stdinFd >= 0 ) {
			close( _stdinFd );
			_stdinFd = -1;
		}
		return -1;
	}

	this->_pid = fork();

	if ( this->_pid < 0 ) {
		Logs::log( LOGS_ERROR, "Fork failed" );
		if ( _stdinFd >= 0 ) {
			close( _stdinFd );
			_stdinFd = -1;
		}
		close( _outputPipe[0] );
		close( _outputPipe[1] );
		_outputPipe[0] = _outputPipe[1] = -1;
		return -1;
	} else if ( this->_pid == 0 ) {

		this->dupCgiFds();

		// build argv
		std::vector< char * > argv;
		argv.push_back( const_cast< char * >( _filePath.c_str() ) );
		argv.push_back( NULL );

		// build envp
		std::vector< char * > envp;
		this->_envp.clear();

		for ( size_t i = 0; i < _envStrings.size(); ++i ) {
			this->_envp.push_back( const_cast< char * >( _envStrings[i].c_str() ) );
		}
		this->_envp.push_back( NULL );

		execve( this->getFilePath().c_str(), argv.data(), this->_envp.data() );
		Logs::log( LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() + "': " + strerror( errno ) );
		_exit( 1 );
	} else {
		if ( _stdinFd >= 0 ) {
			close( _stdinFd );
			_stdinFd = -1;
		}
		close( _outputPipe[1] );
		_outputPipe[1] = -1;
		fcntl( _outputPipe[0], F_SETFL, fcntl( _outputPipe[0], F_GETFL, 0 ) | O_NONBLOCK );
	}
	return 0;
};

IN_OUT_STATE &http::Cgi::getState() {
	return _state;
};

std::string &http::Cgi::getReadBuffer() {
	return _outputBuffer;
};

void http::Cgi::appendOutputToResponse( Response &response ) {
	response.appendCgiChunk( _outputBuffer );
}

void http::Cgi::appendFinalOutputToResponse( Response &response ) {
	response.appendCgiChunk( _outputBuffer, true );
}

bool http::Cgi::readFromPipe() {
	char buffer[BUFFER_SIZE];

	ssize_t bytesReceived = read( getOutputPipeFd(), buffer, BUFFER_SIZE );

	if ( bytesReceived > 0 ) {
		_outputBuffer.append( buffer, bytesReceived );
		_state = READ_SUCCESS;
		return true;
	} else if ( bytesReceived == 0 ) {
		_state = CGI_COMPLETED;
		return false;
	} else {
		Logs::log( LOGS_ERROR, "Error reading from CGI pipe" );
		_state = READ_ERROR;
		return false;
	}
}

void http::Cgi::dumpEnvStrings() const {
	std::cerr << "[CGI envStrings]" << std::endl;
	for ( size_t i = 0; i < _envStrings.size(); ++i ) {
		std::cerr << _envStrings[i] << std::endl;
	}
}

void http::Cgi::dumpEnvp() const {
	std::cerr << "[CGI envp]" << std::endl;
	for ( size_t i = 0; i < _envp.size(); ++i ) {
		if ( _envp[i] == NULL )
			break;
		std::cerr << _envp[i] << std::endl;
	}
}

bool http::Cgi::hasFinished() {

	if ( _hasFinished )
		return true;

	pid_t result = waitpid( _pid, &_status, WNOHANG );
	if ( result > 0 ) {
		_hasFinished = true;
		return true;
	}
	_hasFinished = false;
	return _hasFinished;
}

bool http::Cgi::hasSuccessfullyFinished() const {

	int status = _status;

	if ( WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
		return true;
	return false;
}
