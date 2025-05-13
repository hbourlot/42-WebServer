#pragma once
#include "Configs.hpp"
#include <iostream>
void printConfigs(Configs& config){
	std::vector<Server>::iterator it = config.servers.begin();
	std::vector<Server>::iterator ite = config.servers.end();
	int i = 1;
	while (it != ite){
		
		std::cout << "\033[31m";
		std::cout << "***************************************" << std::endl;
		std::cout << "***********PRINTING SERVER " << i << "***********" << std::endl;
		std::cout << "***************************************\n" << std::endl;
		std::cout << "\033[0m";

		std::cout << "Host: " << it->host << std::endl; 
		std::cout << "Port " << it->port << std::endl; 
		std::cout << "Server_Name: " << it->serverName << std::endl; 
		std::cout << "Max request: " << it->maxRequest << std::endl; 
		
		it++;
		i++;
		std::cout << "\n";
	}
}