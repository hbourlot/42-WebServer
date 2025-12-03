// #include "Config/Debug.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

int main( int argc, char **argv, char **envp ) {

	Configs configuration;
	try
	{

		if ( argc != 2 )
			throw std::invalid_argument( "Invalid number of arguments" );
		
		if ( CheckConf::checkConfExtension(argv[1]) == false)
			throw std::invalid_argument( "Extension has to be '.conf'" );
		
		// Creates a config object
		// Reads the configuration file
		ReadConfig::setConfigs( argv[ 1 ], configuration );
	} catch (const std::exception &exception)
	{
		std::cout << "Failed to set the configurations: " << exception.what() << std::endl;
		return (-1); // If we use exit, we got a leak from exception
	}

	// Namespace to use on TCPServer
	using namespace http;

	TcpServer server( configuration.servers[ 0 ] );

	server.runServer();

	return 0;
}
