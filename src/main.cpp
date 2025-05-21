#include "http_tcpServer_linux.hpp"
#include "Debug.hpp"

int main(int ac, char *av[]){
	if (ac != 2){
		std::cerr << "No confing file in input" << std::endl;
		return -1;
	}

	if (!CheckConfName::checkConfExtension(av[1]) || 
		!CheckConfName::checkConfOpen(av[1]))
			return -1;
	Configs configs;
	if (!ReadConfig::setConfigs(av[1], configs))
		std::cout << "Invalid config file" << std::endl;
	else {
		std::cout << "Valid confing file" << std::endl;
		printConfigs(configs);
	}
	using namespace http;

	TcpServer server("127.0.0.1", 8080);

	server.runServer();

	return 0;
}