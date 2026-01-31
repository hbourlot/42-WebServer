#include "httpTcpServer/Cgi.hpp"

// void printEnvStrings( std::vector< std::string > &_envStrings ) {
// 	std::cerr << "ENVSTRINGS:" << std::endl;
// 	for ( size_t i = 0; i < _envStrings.size(); ++i ) {
// 		std::cerr << "  [" << i << "]: " << _envStrings[ i ] << std::endl;
// 	}
// 	std::cerr << "END ENVSTRINGS" << std::endl;
// }

// void debugCgiExec( const char *filePath, char *const argv[], char *const envp[] ) {
// 	std::cerr << "CGI EXEC DEBUG" << std::endl;
// 	std::cerr << "File path: " << ( filePath ? filePath : "NULL" ) << std::endl;

// 	std::cerr << "ARGV:" << std::endl;
// 	for ( int i = 0; argv && argv[ i ]; ++i ) {
// 		std::cerr << "  argv[" << i << "]: " << argv[ i ] << std::endl;
// 	}

// 	std::cerr << "ENVP:" << std::endl;
// 	for ( int i = 0; envp && envp[ i ]; ++i ) {
// 		std::cerr << "  envp[" << i << "]: " << envp[ i ] << std::endl;
// 	}
// 	std::cerr << "END DEBUG" << std::endl;
// }

// void http::Cgi::executeCgi() {

// 	if ( pipe( this->_inputPipe ) < 0 || pipe( this->_outputPipe ) < 0 ) {
// 		std::cerr << "Pipe creating failed\n";
// 		return;
// 	}

// 	pid_t pid;

// 	_bodyFileName = _filePath + "_" + _client->getSessionId() + "_content.txt";
// 	std::cout <<"_bodyFileName: "<< _bodyFileName << std::endl;
// 	_inputPipe[ 1 ] = open( _bodyFileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644 );
// 	if ( _inputPipe[ 1 ] < 0 )
// 		return; // !!! manage error

// 	_response.buildResponse( HTTP_OK, _request.body );
// 	const std::string &response = _response.buildResponseString();
// 	write( _inputPipe[ 1 ], response.c_str(), response.size() );
// 	close( _inputPipe[ 1 ] );

// 	_inputPipe[ 0 ] = open( _bodyFileName.c_str(), O_RDONLY );
// 	if ( _inputPipe[ 0 ] < 0 )
// 		return; // !!! manage error

// 	this->_pid = fork();

// 	if ( this->_pid < 0 ) {
// 		std::cerr << "Fork failed\n";
// 		return;
// 	} else if ( this->_pid == 0 ) {

// 		this->doDupTwoWay();

// 		// build argv
// 		std::vector< char * > argv;
// 		argv.push_back( const_cast< char * >( _filePath.c_str() ) );
// 		// argv.push_back( const_cast< char* >( "http://localhost:8001/directory/youpi.bla" ) ); // !
// 		argv.push_back( NULL );

// 		// build envp
// 		std::vector< char * > envp;
// 		this->_envp.clear();

// 		for ( size_t i = 0; i < _envStrings.size(); ++i ) {
// 			this->_envp.push_back( const_cast< char * >( _envStrings[ i ].c_str() ) );
// 		}
// 		this->_envp.push_back( NULL );

// 		execve( this->getFilePath().c_str(), argv.data(), this->_envp.data() );
// 		Logs::log( LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() + "': " + strerror( errno ) );
// 		_exit( 1 );
// 	} else {
// 		this->closeForTwoWay();
// 		fcntl( _outputPipe[ 0 ], F_SETFL, O_NONBLOCK );
// 		fcntl( _inputPipe[ 1 ], F_SETFL, O_NONBLOCK );
// 	}
// };