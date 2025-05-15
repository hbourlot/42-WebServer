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
		
		for (std::map<int, std::string>::iterator ep_it = it->errorPage.begin();
			ep_it != it->errorPage.end(); ++ep_it) {
			std::cout << "Code: " << ep_it->first << " | Path: " << ep_it->second << std::endl;
		}

		for (std::vector<Location>::iterator itl = it->locations.begin(); itl != it->locations.end(); ++itl) {
			std::cout << "  Location Path: " << itl->path << std::endl;
			std::cout << "  Root: " << itl->root << std::endl;
			std::cout << "  Methods: ";
			for (int i = 0; itl->methods.size() > i; i++)
				std::cout << itl->methods[i] << " ";
			std::cout << "\n\n";
		}
		// for (std::vector<int, std::string>::iterator it3 = it->errorPage.begin(); it3 != (*it)->errorPage.end(); it++)

		
		it++;
		i++;
		std::cout << "\n";
	}
}