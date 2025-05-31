#include "Config/Debug.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

int main(int argc, char **argv) {

	if (argc != 2)
		throw std::invalid_argument("Error: Invalid number of arguments");

	using namespace http;

	Configs configuration; // Creates a config object
	ReadConfig::setConfigs(argv[1],
						   configuration); // Reads the configuration file
 
	//* std::cout <<"host "<< configuration.servers[0].host << std::endl;
	//* std::cout <<"port "<< configuration.servers[0].port << std::endl;
	//* std::cout <<"serverName "<< configuration.servers[0].serverName << std::endl;
	//* std::cout <<"error_page 404 "<< configuration.servers[0].errorPage[404] << std::endl;
	//* std::cout <<"error_page 500 "<< configuration.servers[0].errorPage[500] << std::endl;
	//* std::cout <<"location[1].path "<< configuration.servers[0].locations[1].path << std::endl;
	//* std::cout <<"location[1].root "<< configuration.servers[0].locations[1].path << std::endl;

	
	// std::vector<TcpServer> servers;

	// configuration.server[0];
	// configuration.server[1];

	// servers[0] = configuration.server[0]
	// servers[1] = configuration.server[1]

	// for (int i = 0; i < configuration.servers.size(); i++){
	// 	TcpServer server(configuration.servers[i]);
	// 	servers.push_back(server);
	// }

	TcpServer server(configuration);

	server.runServer();

	return 0;
}
