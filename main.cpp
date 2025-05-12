#include "webserver.hpp"
#include "server.hpp"
#include "CheckConfName.hpp"
#include "ReadConfig.hpp"
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
	
	ReadConfig::setConfigs(av[1]);
	std::cout << "Valid confing file" << std::endl;
}