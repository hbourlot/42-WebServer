#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include "Config/Debug.hpp"
// CRUD : create, read, update, delete

#include "http_tcpServer/http_tcpServer_linux.hpp"

int main(int argc, char **argv) {

	if (argc != 2)
		throw std::invalid_argument("Error: Invalid number of arguments");

	using namespace http;

	Configs configuration; // Creates a config object
	ReadConfig::setConfigs(argv[1], configuration); // Reads the configuration file

	// std::vector<TcpServer> servers;

	//configuration.server[0];
	//configuration.server[1];

	//servers[0] = configuration.server[0]
	//servers[1] = configuration.server[1]

	// for (int i = 0; i < configuration.servers.size(); i++){
	// 	TcpServer server(configuration.servers[i]);
	// 	servers.push_back(server);
	// }

	TcpServer server(configuration);

	server.runServer();

	return 0;
}

// int main(int ac, char *av[]){
// 	if (ac != 2){
// 		std::cerr << "No confing file in input" << std::endl;
// 		return -1;
// 	}

// 	if (!CheckConf::checkConfExtension(av[1]) || 
// 		!CheckConf::checkConfOpen(av[1]))
// 			return -1;
// 	Configs configs;
// 	if (!ReadConfig::setConfigs(av[1], configs))
// 		std::cout << "Invalid config file" << std::endl;
// 	else {
// 		std::cout << "Valid confing file" << std::endl;
// 		printConfigs(configs);
// 	}
	
// }