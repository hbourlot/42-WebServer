#include "Config/ReadConfig.hpp"
#include "Config/SetLocations.hpp"
#include <string>

#define HOST 1
#define PORT 2
#define SERVER_NAME 3
#define CLIENT_MAX_BDY 4
#define ERROR_PAGE 5
#define LOCATION 6

Server::Server(){
	port = 0;
	maxRequest = 0;
}

void 	getErrorPage(std::string noSpaceLine, Server& server){
	std::istringstream iss(noSpaceLine);
    std::string directive;
    std::string code;
    std::string path;

    iss >> directive >> code >> path;

	for (int i = 0; code[i]; i++){ // Checks if the code has only numbers
		if (!isdigit(code[i]))
			throw std::invalid_argument("Error: Route code is invalid\n");
	}

    if (!path.empty() && path[path.size() - 1] == ';') // Removes the ';'
        path.erase(path.size() - 1);

	server.errorPage.insert(std::pair<int, std::string>(std::atoi(code.c_str()), path));

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
	else if (trimedLine == "location")
		return LOCATION;
	return 7;
}

bool	ReadConfig::setServerConfig(std::ifstream& confFd, std::string& line, Configs& configs){
	std::string noSpaceLine; // Gets the string without the initial spaces	
	std::string trimedLine; // Stores the atribute of the server
	Server server; // Variable to save all the information
	
	server.port = 0;
	server.maxRequest = 10; // Set the max value by default
	while (std::getline(confFd, line)){ // Finish the server config block
		noSpaceLine = removeSpace(line); // Removes the first spaces
		
		if (!CheckConf::checkLineFinished(noSpaceLine))
			throw std::invalid_argument("Error: Extra words after End of Line\n");
			
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
			
			case LOCATION:
				SetLocation::setLocationConfig(confFd, noSpaceLine.substr(noSpaceLine.find(' ')), server);
				break;

			default:
				break;
		}
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));
	}
	ReadConfig::setDefaultServer(server);
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

void	ReadConfig::setDefaultServer(Server& server){
	if (server.host.empty()){
		std::cout << "Setting default host 127.0.0.1 ✅" << std::cout;
		server.host = "127.0.0.1";
	}
	
	if (server.port == 0){
		std::cout << "Server port 0 -- Setting to 8080 ✅" << std::endl;
		server.port = 8080;
	}

	if (server.serverName.empty()){
		std::cout << "Setting the Server Name to 'Default' ✅" << std::endl;
		server.serverName = "Default";
	}

	if (server.errorPage.empty()){
		std::cout << "Setting default error page information ✅" << std::endl;
		server.errorPage.insert(std::pair<int, std::string> (404, "../../content/defaultError.html"));
	}
	
	if (server.maxRequest == 0){
		std::cout << "Settig Max request to 10M ✅" << std::endl;
		server.maxRequest = 10;
	}
}
