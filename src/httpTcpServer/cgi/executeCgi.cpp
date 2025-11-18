#include "Config/Configs.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <ios>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void printEnvStrings( std::vector< std::string > &_envStrings ) {
	std::cerr << "ENVSTRINGS:" << std::endl;
	for ( size_t i = 0; i < _envStrings.size(); ++i ) {
		std::cerr << "  [" << i << "]: " << _envStrings[ i ] << std::endl;
	}
	std::cerr << "END ENVSTRINGS" << std::endl;
}

void debugCgiExec( const char *filePath, char *const argv[], char *const envp[] ) {
	std::cerr << "CGI EXEC DEBUG" << std::endl;
	std::cerr << "File path: " << ( filePath ? filePath : "NULL" ) << std::endl;

	std::cerr << "ARGV:" << std::endl;
	for ( int i = 0; argv && argv[ i ]; ++i ) {
		std::cerr << "  argv[" << i << "]: " << argv[ i ] << std::endl;
	}

	std::cerr << "ENVP:" << std::endl;
	for ( int i = 0; envp && envp[ i ]; ++i ) {
		std::cerr << "  envp[" << i << "]: " << envp[ i ] << std::endl;
	}
	std::cerr << "END DEBUG" << std::endl;
}

bool http::Cgi::readCgiOutput( void ( *updateStatusPtr )() ) {

	char buffer[ http::BUFFER_SIZE + 1 ] = { 0 };
	const int MAX_READS_PER_EVENT = 3;
	int readCount = 0;

	while ( readCount < MAX_READS_PER_EVENT ) {
		ssize_t bytesReceived = read( this->_outputPipe[ 0 ], buffer, BUFFER_SIZE );

		if ( bytesReceived > 0 ) {
			_body.append( buffer, bytesReceived ); // Only append received bytes
			readCount++;
			std::memset( buffer, 0, BUFFER_SIZE + 1 );
			continue;
		}

		if ( bytesReceived == 0 && readCount == 0 ) {
			return true;
		}

		if ( errno == EAGAIN || errno == EWOULDBLOCK )
			return true;

		if ( bytesReceived == 0 && readCount != 0 )
			break; // ! Need to make sure if makes sense

		return false;
	}
	if ( updateStatusPtr )
		updateStatusPtr();

	return true;
}

void http::Cgi::handleChildProcess() {

	doDupOneWay();

	this->_filePath = "." + this->_filePath;

	// build argv
	std::vector< char * > argv;
	argv.push_back( const_cast< char * >( _filePath.c_str() ) );
	argv.push_back( NULL );

	// build envp
	std::vector< char * > envp;
	this->_envp.clear();

	for ( size_t i = 0; i < _envStrings.size(); ++i ) {
		this->_envp.push_back( const_cast< char * >( _envStrings[ i ].c_str() ) );
	}
	this->_envp.push_back( NULL );

	this->_filePath = "./webpage/cgi-bin/hello.py";

	// Execute the actual CGI script with proper environment
	execve( this->getFilePath().c_str(), argv.data(), this->_envp.data() );
	// execve( this->getFilePath().c_str(), argv.data(), envp.data() );
	std::cerr << "EXECUTE WRONG\n ";
	_exit( 1 );
}

void http::Cgi::executeCgi( std::vector< pollfd > &fds ) { // Working on

	if ( pipe( this->_inputPipe ) < 0 || pipe( this->_outputPipe ) < 0 ) {
		std::cerr << "Pipe creating failed\n";
		return;
	}
	this->_pid = fork();
	if ( this->_pid < 0 ) {
		std::cerr << "Fork failed\n";
		return;
	} else if ( this->_pid == 0 ) {
		this->handleChildProcess();
	} else {
		this->closeForOneWay();

		// int status;
		// waitpid( _pid, &status, 0 );

		// Set both pipes to non-blocking if doing async I/O
		// fcntl( _inputPipe[ 1 ], F_SETFL, O_NONBLOCK );
		// std::cout << "_input[1] : " << _inputPipe[1] << std::endl;
		fcntl( _outputPipe[ 0 ], F_SETFL, O_NONBLOCK );

		// this->readCgiOutput();

		// char buffer[ BUFFER_SIZE ] = { 0 };
		// ssize_t bytes = read( _outputPipe[ 0 ], buffer, BUFFER_SIZE );
		// _body = buffer;

		// // buffer[bytes] = '\0'; // Safely null-terminate
		// std::cout << "[BUFFER]: " << buffer << std::endl;
		// std::cout << "[BUFFER]: " << _body << std::endl;

		// fcntl(_outputPipe[0], F_SETFL,
		//       fcntl(_outputPipe[0], F_GETFL, 0) | O_NONBLOCK);
		this->registerPollFd( fds );
	}
}