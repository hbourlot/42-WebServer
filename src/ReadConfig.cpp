#include "ReadConfig.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

std::string removeSpace(std::string& line){
	int i = 0;

	for (i = 0; line[i] && line[i] == ' '; i++){ // Runs all spaces and return the string without the first spaces
		continue;
	}

	return line.substr(i);
}

std::string	getInfo(std::string& noSpaceLine){
	int i;

	for (i = noSpaceLine.find(' '); noSpaceLine[i] == ' '; i++) // This will remove extra spaces after attribute ex: "port      8080"
		continue;

	if (noSpaceLine.find(';') == std::string::npos) // If don't find the ';' throw an error
		throw std::invalid_argument("Error: Invalid end of line, missing ';' at the end\n");
	
	return noSpaceLine.substr(i, noSpaceLine.find(';') - i); // '- i' I have to discard the 'i' size
}

bool	ReadConfig::setServerConfig(std::ifstream& confFd, std::string& line, Configs& configs){
	std::string noSpaceLine; // Gets the string without the initial spaces	
	std::string trimedLine; // Stores the atribute of the server
	Server server;
	
	server.maxRequest = 10; // Set the max value by default

	while (std::getline(confFd, line)){ // Finish the server config block
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));
		
		if (noSpaceLine[0] == '}') // Checks if it is the closed brackets
			break;
		if (trimedLine == "host") // Set the host information for the server
			server.host = getInfo(noSpaceLine); // Get the information in string
		else if (trimedLine == "port")
			server.port = std::atoi(getInfo(noSpaceLine).c_str()); // Convert the string into a int
		else if (trimedLine == "server_name")
			server.serverName = getInfo(noSpaceLine); // Gets the server name
		else if (trimedLine == "client_max_body_size"){
			if (getInfo(noSpaceLine) != "10M") // Verify if the value is valid
				throw std::invalid_argument("Error: Invalid client_max_body, the only one is 10\n");
		}
		else if (trimedLine == "error_page"){
			// get error_page
			std::cout << "Found an error_page" << std::endl;
		}
	}
	configs.servers.push_back(server);
	return (true);
}

bool    ReadConfig::setConfigs(char* conf, Configs& configs){
    bool    		inServer = false;
    std::ifstream 	confFd;
	std::string		line;
	confFd.open(conf); // Open the config file.
	
	while(std::getline(confFd, line)){
		if (removeSpace(line) == "server {"){ // Removes the spaces before the name and return the value to check if it is a server
			if (!ReadConfig::setServerConfig(confFd, line, configs)) // Will check if everything is OK when we get the server config info
				return (false);
		}
	}
    confFd.close();
	return (true);
};