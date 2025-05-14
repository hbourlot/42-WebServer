#include "ReadConfig.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

#define HOST 1
#define PORT 2
#define SERVER_NAME 3
#define CLIENT_MAX_BDY 4
#define ERROR_PAGE 5
#define ROUTE 6

#define METHODS 7
#define ROOT 8

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

std::string routePath(std::string& line){
	int i;
	
	for (i = 0; line[i] && line[i] == ' '; i++) //Skip the spaces
		continue;
	
	int j;
	for (j = 0; line[j] && line[j] != ' ' && line[j] != '{'; j++) // Gets the path size
		continue;

	return line.substr(i,j + 1); // Return the path
}

void	getMethods(std::string trimedLine, std::vector<std::string>& methods){ // Function to get the route methods
	while (trimedLine.find(' ') != std::string::npos){ // Loops until has no more spaces
		trimedLine = trimedLine.substr(0, trimedLine.find(' ')); // Gets the method
		methods.push_back(trimedLine); // Push the method to the vector
	}
}

int		getTypeRoute(std::string& trimedLine){ // Function to check the information to set
	if (trimedLine == "methods")
		return METHODS;
	if (trimedLine == "root")
		return ROOT;

	return 10;
}

bool	ReadConfig::setRouteConfig(std::ifstream& confFd, std::string line, Server& server){
	std::string noSpaceLine; // Gets the string without the initial spaces	
	std::string trimedLine; // Stores the atribute of the route
	Route route;

	route.path = routePath(line); // Sets the route path

	while (std::getline(confFd, line)){
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));

		if (trimedLine[0] == '}')
			break;
		
		switch(getTypeRoute(trimedLine)){
			case METHODS:
				getMethods(trimedLine, route.methods);
				break;
			case ROOT:
				route.root = getInfo(noSpaceLine);
				break;
			default:
				break;

		}
	}
	(void)noSpaceLine;
	(void)trimedLine;
	(void)route;
	(void)confFd;
	(void)server;
	server.routes.push_back(route);
	return true;
}

void 	getErrorPage(std::string noSpaceLine, Server& server){
	int i = 0;
	
	while (server.errorPage.inser) // <------------ FIQUEI AQUI 
}

int		getTypeServer(std::string& trimedLine){ // Return the type of information to use on switch
	if (trimedLine == "host")
		return HOST;
	else if (trimedLine == "port")
		return PORT;
	else if (trimedLine == "server_name")
		return SERVER_NAME;
	else if (trimedLine == "client_max_body_size")
		return CLIENT_MAX_BDY;
	else if (trimedLine == "error_page")
		return ERROR_PAGE;
	else if (trimedLine == "route")
		return ROUTE;
	return 7;
}

bool	ReadConfig::setServerConfig(std::ifstream& confFd, std::string& line, Configs& configs){
	std::string noSpaceLine; // Gets the string without the initial spaces	
	std::string trimedLine; // Stores the atribute of the server
	Server server; // Variable to save all the information
	
	server.maxRequest = 10; // Set the max value by default
	while (std::getline(confFd, line)){ // Finish the server config block
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));

		if (trimedLine[0] == '}') // Finish the server info
			break;

		switch(getTypeServer(trimedLine)){
			case HOST:
				server.host = getInfo(noSpaceLine); // Get the information in string
				break;
	
			case PORT:
				server.port = std::atoi(getInfo(noSpaceLine).c_str()); // Convert the string into a int
				break;
		
			case SERVER_NAME:
				server.serverName = getInfo(noSpaceLine); // Gets the server name
				break;
			
			case CLIENT_MAX_BDY:
				if (getInfo(noSpaceLine) != "10M") // Verify if the value is valid
					throw std::invalid_argument("Error: Invalid client_max_body, the only one is 10\n");
				break;
			
			case ERROR_PAGE:
				getErrorPage(noSpaceLine, server);
				break;
			
			case ROUTE:
				setRouteConfig(confFd, noSpaceLine.substr(noSpaceLine.find(' ')), server);
				break;

			default:
				break;
		}
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));
	}
	configs.servers.push_back(server); // Send the information for the main config
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