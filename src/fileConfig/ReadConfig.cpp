#include "Config/ReadConfig.hpp"
#include "Config/SetFile.hpp"
#include "Config/SetLocations.hpp"
#include <string>
#include <utils.hpp>

#define HOST 1
#define PORT 2
#define SERVER_NAME 3
#define CLIENT_MAX_BDY 4
#define ERROR_PAGE 5
#define LOCATION 6
#define ROOT 7
#define FILE 8
#define INDEX 9
#define CGIPASS 10
#define TEMP_PATH 11
#define BODY_BUFFER 12

ServerConfig::ServerConfig() {
	port = 0;
	max_body_size = 0;
}

Directory *ServerConfig::GetLocationByPath(std::string path) {
	for (size_t curLocationIdx = 0; curLocationIdx < directories.size(); curLocationIdx++) {
		if (directories[curLocationIdx].path == path) {
			return &directories[curLocationIdx];
		}
	}
	return NULL;
}

File *ServerConfig::GetFileByExtension(std::string extension) { // Get the file by extension ".bat" (For example)
	for (size_t curFileIdx = 0; curFileIdx < files.size(); curFileIdx++) {
		if (files[curFileIdx].extension == extension) {
			return &files[curFileIdx];
		}
	}
	return NULL;
}

void getErrorPage(std::string noSpaceLine, ServerConfig &server) {
	std::istringstream iss(noSpaceLine);
	std::string directive;
	std::string code;
	std::string path;

	iss >> directive >> code >> path;

	for (int i = 0; code[i]; i++) { // Checks if the code has only numbers
		if (!isdigit(code[i]))
			throw std::invalid_argument("Error: Route code is invalid\n");
	}

	if (!path.empty() && path[path.size() - 1] == ';') // Removes the ';'
		path.erase(path.size() - 1);

	server.errorPage.insert(std::pair<int, std::string>(std::atoi(code.c_str()), path));
}

int getTypeServer(std::string &trimmedLine) { // Return the type of information to use on switch
	if (trimmedLine == "host")
		return HOST;
	else if (trimmedLine == "port")
		return PORT;
	else if (trimmedLine == "server_name")
		return SERVER_NAME;
	else if (trimmedLine == "client_max_body_size")
		return CLIENT_MAX_BDY;
	else if (trimmedLine == "error_page")
		return ERROR_PAGE;
	else if (trimmedLine == "location")
		return LOCATION;
	else if (trimmedLine == "root")
		return ROOT;
	else if (trimmedLine == "index")
		return INDEX;
	else if (trimmedLine == "cgi_pass")
		return CGIPASS;
	else if (trimmedLine == "client_body_temp_path")
		return TEMP_PATH;
	else if (trimmedLine == "client_body_buffer_size")
		return BODY_BUFFER;
	return 100;
}

bool ReadConfig::setServerConfig(std::ifstream &confFd, std::string &line, Configs &configs) {
	std::string noSpaceLine; // Gets the string without the initial spaces
	std::string trimmedLine; // Stores the attribute of the server
	std::string emptyString; // Just an empty string
	ServerConfig server;     // Variable to save all the information

	bool IsServerOpen = false;

	// Check if the first line opens the brackets or not
	if (containBrackets(line, IsServerOpen, emptyString) == false) {
		return false;
	}

	server.port = 0;
	server.max_body_size = 0; // Set the max value by default
	server.max_buffer_size = 0;
	while (std::getline(confFd, line)) { // Finish the server config block

		noSpaceLine = removeSpace(line); // Removes the first spaces

		if (!CheckConf::checkLineFinished(noSpaceLine))
			throw std::invalid_argument("Error: Extra words after End of Line\n");

		trimmedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));

		if (trimmedLine[0] == '}') // Finish the server info
			break;

		// Check if we are going to location configs "location /upload"
		// This serves to check if we have stuff like " } location /upload {"
		// We close the last location config, and we open a new one
		if (line.find("location") == std::string::npos) {

			if (containBrackets(line, IsServerOpen, emptyString) == false)
				return false;
		}

		else {
			if (checkSplitString(line, "location", IsServerOpen) == false)
				return false;
		}

		if (IsServerOpen == true) {
			switch (getTypeServer(trimmedLine)) {
			case ROOT:
				server.root = getInfo(noSpaceLine); // Get the root path
				break;
			case HOST:
				server.host = getInfo(noSpaceLine); // Get the information in string
				break;
			case INDEX:
				server.index = getInfo(noSpaceLine);
				break;
			case PORT:
				server.port = std::atoi(getInfo(noSpaceLine).c_str()); // Convert the string into a int
				break;

			case SERVER_NAME:
				server.serverName = getInfo(noSpaceLine); // Gets the server name
				break;

			case CLIENT_MAX_BDY:
				server.max_body_size = getMaxRequestBody(noSpaceLine);
				if (server.max_body_size == -1) {
					std::cerr << "Invalid suffix of max body size" << std::endl;
					return false;
				}
				break;

			case ERROR_PAGE:
				getErrorPage(noSpaceLine, server);
				break;

			case FILE:
				if (SetFile::setFileConfig(confFd, noSpaceLine.substr(noSpaceLine.find(' ')), server) == false)
					return false;
				break;

			case TEMP_PATH:
				server.temp_path = getInfo(noSpaceLine);
				std::cout << "Temp path " << server.temp_path << std::endl;
				break;

			case BODY_BUFFER:
				server.max_buffer_size = getMaxRequestBody(noSpaceLine);
				if (server.max_body_size == -1) {
					std::cerr << "Invalid suffix of max body size" << std::endl;
					return false;
				}
				std::cout << "Buffer size " << server.max_buffer_size << std::endl;
				break;
			case LOCATION:
				if (SetLocation::setLocationConfig(confFd, noSpaceLine.substr(noSpaceLine.find(' ')), server) ==
				    false) {
					return false;
				}
				break;

			default:
				break;
			}
		}
		noSpaceLine = removeSpace(line);
		trimmedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));
	}
	ReadConfig::setDefaultServer(server);
	configs.servers.push_back(server); // Send the information for the main config
	return (true);
}

bool ReadConfig::setConfigs(char *conf, Configs &configs) {
	bool inServer = false;
	std::ifstream confFd;
	std::string line;
	confFd.open(conf); // Open the config file.
	try {
		while (std::getline(confFd, line)) {
			if (line.empty() == false &&
			    containBrackets(line, inServer, "server") ==
			        true) { // Removes the spaces before the name and return the value to check if it is a server
				if (ReadConfig::setServerConfig(confFd, line, configs) ==
				    false) // Will check if everything is OK when we get the server config info
				{
					return (false);
				}
			}
		}
	} catch (const std::exception &exception) {
		std::cerr << "Got an exception " << exception.what() << std::endl;
		confFd.close();
	}
	confFd.close();
	if (configs.servers.empty() == true)
		return false;

	ReadConfig::setServerLocations(configs);
	return (true);
};

void ReadConfig::setDefaultServer(ServerConfig &server) {
	if (server.host.empty()) {
		std::cout << "Setting default host 127.0.0.1 ✅" << std::endl;
		server.host = "127.0.0.1";
	}

	if (server.port == 0) {
		std::cout << "Server port 0 -- Setting to 8080 ✅" << std::endl;
		server.port = 8080;
	}

	if (server.serverName.empty()) {
		std::cout << "Setting the Server Name to 'Default' ✅" << std::endl;
		server.serverName = "Default";
	}

	if (server.errorPage.empty()) {
		std::cout << "Setting default error page information ✅" << std::endl;
		server.errorPage.insert(std::pair<int, std::string>(404, "../../content/defaultError.html"));
	}

	if (server.max_body_size == 0) {
		std::cout << "Setting Max request to 1024 ✅" << std::endl;
		server.max_body_size = 1024;
	}

	if (server.max_buffer_size == 0) {
		std::cout << "Setting max buffer size to 1MB ✅" << std::endl;
		server.max_buffer_size = 1024 * 1024;
		for (int i = 0; i < server.directories.size(); ++i) {
			server.directories[i].max_buffer_size = server.directories[i].max_buffer_size == 0 ? server.max_buffer_size : server.directories[i].max_buffer_size;
		}
		for (int i = 0; i < server.files.size(); ++i) {
			server.files[i].max_buffer_size = server.files[i].max_buffer_size == 0 ? server.max_buffer_size : server.files[i].max_buffer_size;
		}
	}

	if (server.temp_path.empty()) {
		std::cout << "Setting the temp path to project root ✅" << std::endl;
		server.temp_path = "./";
	}
}

static void copyDirectoryIntoLocation(Directory &dir, Location &loc) {

	// Core routing
	loc.path = dir.path;
	loc.methods = dir.methods;

	// Serving
	loc.root = dir.root;
	loc.index = dir.index;
	loc.autoIndex = dir.autoIndex;

	// Redirects
	loc.redirection = dir.redirection;

	// CGI
	loc.cgi_pass = dir.cgi_pass;
	loc.cgi_extension = dir.cgi_extension;
	loc.cgi_path = dir.cgi_path;
	loc.cgi = dir.cgi;

	// Upload
	loc.uploadEnable = dir.uploadEnable;
	loc.uploadStore = dir.uploadStore;

	// Limits
	loc.max_body_size = dir.max_body_size;
	loc.max_buffer_size = dir.max_buffer_size;

	loc.type |= LOCATION_DIR;
	if (!loc.cgi_extension.empty() || !loc.cgi_pass.empty())
		loc.type |= LOCATION_CGI;
	if (loc.uploadEnable)
		loc.type |= LOCATION_UPLOAD;
	if (!loc.redirection.empty())
		loc.type |= LOCATION_REDIRECT;
}

static void copyFileIntoLocation(File &file, Location &loc) {

	loc.extension = file.extension;
	loc.methods = file.methods;
	loc.root = file.root;
	loc.index = file.index;
	loc.cgi_pass = file.cgi_pass;
	loc.max_buffer_size = file.max_buffer_size;

	loc.type |= LOCATION_FILE;
}

void ReadConfig::setServerLocations(Configs &configs) {

	for (size_t i = 0; i < configs.servers.size(); ++i) {

		ServerConfig &server = configs.servers[i];
		std::vector<Location> loc;

		for (size_t j = 0; j < server.directories.size(); ++j) {
			Location l;
			copyDirectoryIntoLocation(server.directories[j], l);
			loc.push_back(l);
		}

		for (size_t j = 0; j < server.files.size(); ++j) {
			Location l;
			copyFileIntoLocation(server.files[j], l);
			loc.push_back(l);
		}

		server.locations = loc;
	}
}
