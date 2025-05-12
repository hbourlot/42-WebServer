#include "ReadConfig.hpp"
#include <fstream>
#include <string>
#include <sstream>

std::string removeSpace(std::string& line){
	int i;
	for (i = 0; line[i]; i++)
		continue;
	return line.substr(line[i]);
}

bool    ReadConfig::setConfigs(char* conf){
    bool    		inServer = false;
    std::ifstream 	confFd;
	std::string		line;
	confFd.open(conf); // Open the config file.
	
	while(std::getline(confFd, line)){
		if (removeSpace(line) == "server {"){ // Removes the spaces before the name and return the value to check if it is a server
			getServerInfo()
		}
	}
    confFd.close();
	return (true);
};