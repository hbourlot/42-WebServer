// #include "Config/Debug.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"
#include "../inc/Config/Debug.hpp"

class ServerConfig;
// Namespace to use on TCPServer
using namespace http;

void cleanup(int signo) {
	getStopServer() = true;
}

bool &getStopServer() {
	static bool s_stopServer = false;
	return (s_stopServer);
}


std::vector< TcpServer * > initialize_all_servers( const Configs &configuration ) {

	std::vector< TcpServer * > servers;

	for ( size_t i = 0; i < configuration.servers.size(); ++i ) {
		// std::cout << "------------------ SERVER " << i << "\n";
		TcpServer *serv = new TcpServer( configuration.servers[i] );
		// std::cout << "keep -> " << configuration.servers[i].alive_timeout << std::endl;
		// printFiles(configuration.servers[i].files);
		// printDirectories(configuration.servers[i].directories);
		serv->startServer();
		servers.push_back( serv );
	}

	return servers;
};

std::vector< pollfd > get_all_listening_sockets_from( const std::vector< TcpServer * > &servers ) {

	std::vector< pollfd > allSocketsFD;
	for ( size_t i = 0; i < servers.size(); i++ ) {
		allSocketsFD.push_back( servers[i]->getServerPOLLFD() );
	}

	return allSocketsFD;
};

void free_servers_memory( const std::vector< TcpServer * > &servers ) {
	for ( size_t i = 0; i < servers.size(); ++i ) {
		delete servers.at( i );
	}
}

int main( int ac, char **av ) {

		struct sigaction sa;
	sa.sa_handler = cleanup;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	Configs configuration;

	try {

		if ( ac != 2 )
			throw std::invalid_argument( "Invalid number of arguments" );

		if ( CheckConf::checkConfExtension( av[1] ) == false )
			throw std::invalid_argument( "Extension has to be '.conf'" );

		if ( ReadConfig::setConfigs( av[1], configuration ) == false )
			throw std::invalid_argument( "Failed to set the configs" );

	} catch ( const std::exception &exception ) {
		std::cout << "Failed to initialize servers: " << exception.what() << std::endl;
		return ( -1 );
	}

	std::vector< TcpServer * > servers = initialize_all_servers( configuration );
	std::vector< pollfd > all_servers_fds = get_all_listening_sockets_from( servers );
	ClientEventProcessor handleEvents( all_servers_fds, servers );

	handleEvents.run();
	
	free_servers_memory( servers );

	return 0;
}
