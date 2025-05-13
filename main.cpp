#include "webserver.hpp"
#include "server.hpp"
#include "CheckConfName.hpp"
#include "ReadConfig.hpp"
#include "Debug.hpp"
// CRUD : create, read, update, delete

// int	main(int ac, char *av[])
// {
// 	Server server;
// 	(void)server;
// }

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
	else 
		std::cout << "Valid confing file" << std::endl;
	printConfigs(configs);
	
}