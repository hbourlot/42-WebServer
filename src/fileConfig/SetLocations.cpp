#include "Config/SetFile.hpp"
#include "Config/SetLocations.hpp"
#include <utils.hpp>


class SetFile;

// Directory::Directory() {
// 	uploadEnable = false;
// 	autoIndex = false;
// 	max_body_size = 0;
// 	// String are automatically initialized;
// }
Directory::Directory()
    : name(""), path(""), methods(), root(""), index(""), redirection(""), cgi_extension(), cgi_path(), cgi(),
      cgi_pass(""), max_body_size(0), max_buffer_size(0), uploadEnable(false), uploadStore(""), autoIndex(false),
      next(), auth_login_page ("/login"), auth(false) {
	// Safety initialization
}

std::string locationPath(const std::string &line) {
	size_t start = line.find('/');  // Start where we have to find the path
	if (start == std::string::npos) // If doesn't find any start returns null
		return "";

	size_t end = start;
	while (end < line.size() && !std::isspace(line[end]) &&
	       line[end] != '{') // We count the end of the path until we find a 'space' or '{', for cases like
	                         // "location /cgi bin/hello"
	{
		end++;
	}

	return line.substr(start, end - start);
}

void SetLocation::getMethods(std::string noSpaceLine,
                             std::vector<std::string> &methods) { // Function to get the Location methods
	std::istringstream iss(noSpaceLine);
	std::string method;

	iss >> method;          // Skip the method word
	while (iss >> method) { // Saves the new method
		if (!method.empty() && method[method.size() - 1] == ';')
			method.erase(method.size() - 1);

		if (method != "GET" && method != "POST" && method != "DELETE") // Check if the method is valid
			throw std::invalid_argument("Error: Invalid method!!! Use only (GET/POST/DELETE)");

		methods.push_back(method); // Send it for the method variable
	}
}

int getTypeLocation(std::string &trimmedLine) { // Function to check the information to set
	if (trimmedLine == "methods")
		return METHODS;
	if (trimmedLine == "root")
		return ROOT;
	if (trimmedLine == "redirect")
		return REDIRECT;
	if (trimmedLine == "cgi_extension")
		return CGI_EXTENSION;
	if (trimmedLine == "cgi_path")
		return CGI_PATH;
	if (trimmedLine == "upload_enable")
		return UPLOAD_ENABLE;
	if (trimmedLine == "upload_store")
		return UPLOAD_STORE;
	if (trimmedLine == "autoindex")
		return AUTOINDEX;
	if (trimmedLine == "index")
		return INDEX;
	if (trimmedLine == "cgi_pass")
		return CGIPASS;
	if (trimmedLine == "client_max_body_size")
		return CLIENT_MAX_BDY;
	if (trimmedLine == "client_body_buffer_size")
		return BODY_BUFFER;
	if (trimmedLine[0] == '#')
		return COMMENT;
	if (trimmedLine == "auth_login_page")
		return AUTH_LOGIN;
	if (trimmedLine == "auth")
		return AUTH;
	if (trimmedLine == "{" || trimmedLine == "}" || trimmedLine.size() == 1)
		return EMPTY;
	return 100;
}

void buildCgi(Directory &location) {
	size_t sizeCgiPath = location.cgi_extension.size(); // Get the size of the cgi paths vector
	size_t sizeCgiExtension = location.cgi_path.size(); // Get the size of the cgi extensions vector

	if (sizeCgiExtension != sizeCgiPath) // Check the size of each containers to see if they match
		throw std::invalid_argument("Error: Not the same number as CGI paths and extensions\n");

	for (size_t i = 0; sizeCgiExtension > i && sizeCgiPath > i; i++) // Insert the information on the map
		location.cgi.insert(
		    std::make_pair(location.cgi_extension[i], location.cgi_path[i])); // I have to make a pair to accept

	// Check for dup information maybe
}

int getCgi(std::string noSpaceLine, Directory &location, int cgiInfo) {
	std::istringstream iss(noSpaceLine);
	std::string info;

	iss >> info;

	int ready = 0;
	if (!location.cgi_extension.empty() ||
	    !location.cgi_path.empty()) // It will check if we already have any pre requisites to build the cgi
		ready = 1;

	while (iss >> info) { // Saves the new info

		if (!info.empty() && info[info.size() - 1] == ';')
			info.erase(info.size() - 1);

		if (cgiInfo == CGI_EXTENSION)
			location.cgi_extension.push_back(info); // Send it for the cgi_extension variable
		else
			location.cgi_path.push_back(info); // Send it for the cgi_path variable
	}

	return ready; // If 0, not enough information | If 1, ready to build the map
}

bool SetLocation::setLocationConfig(std::ifstream &confFd, std::string line, ServerConfig &server) {
	std::string noSpaceLine; // Gets the string without the initial spaces
	std::string trimmedLine;  // Stores the attribute of the Location
	std::string emptyString;
	Directory location;
	bool IsLocationOpen = false;

	if (line.find("*.") != std::string::npos) {
		return SetFile::setFileConfig(confFd, line, server);
	}
	location.path = locationPath(line); // Sets the Location path

	if (location.path.size() == 0)
		return false;

	int atIndexFlag = 0; // Setup a flag for autoIndex, to check for CGI
	if (containBrackets(line, IsLocationOpen, emptyString) == false) {
		return false;
	}
	while (std::getline(confFd, line)) {
		noSpaceLine = removeSpace(line);

		if (!CheckConf::checkLineFinished(noSpaceLine)) // Checks if have more information after the limiter
			throw std::invalid_argument("Error: Extra words after End of Line\n");

		trimmedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));
		if (line.find("location") == std::string::npos) {
			if (containBrackets(line, IsLocationOpen, emptyString) == false) {
				return false;
			}
		}

		else {
			if (checkSplitString(line, "location", IsLocationOpen) == false) {

				return false;
			}
		}

		if (trimmedLine[0] == '}')
			break;

		switch (getTypeLocation(trimmedLine)) {
			case METHODS:
				getMethods(noSpaceLine, location.methods);
				break;
			case ROOT:
				location.root = getInfo(noSpaceLine);
				break;
			case REDIRECT:
				location.redirection = getInfo(noSpaceLine);
				break;
			case CGI_EXTENSION:
				location.name = "cgi"; // Used to check on AUTOINDEX
				if (getCgi(noSpaceLine, location, CGI_EXTENSION) == 1)
					buildCgi(location); // -> Missing function
				break;
			case CGI_PATH:
				location.name = "cgi"; // Used to check on AUTOINDEX
				if (getCgi(noSpaceLine, location, CGI_PATH) == 1)
					buildCgi(location); // If we already have the full information (PATH + EXTENSION), we build the map cgi
				break;
			case UPLOAD_ENABLE:
				if (getInfo(noSpaceLine) == "on") // Change the permission to upload files
					location.uploadEnable = true;
				else
					location.uploadEnable = false;
				break;
			case UPLOAD_STORE:
				location.uploadStore = getInfo(noSpaceLine);
				break;
			case AUTOINDEX:
				atIndexFlag = 1; // This will activate a flag, to see if it have any autoIndex inside the location, can't
								// have inside CGI;

				if (getInfo(noSpaceLine) == "on") // Change the permission to upload files
					location.autoIndex = true;
				else if (getInfo(noSpaceLine) == "off")
					location.autoIndex = false;
				else{
					std::cerr << "The auto index must be 'on' or 'off'" << std::endl;
					return false;
				}
				break;
			case INDEX:
				location.index = getInfo(noSpaceLine);
				break;

			case CGIPASS:
				location.cgi_pass = getInfo(noSpaceLine);
				struct stat buffer;
				if (stat(location.cgi_pass.c_str(), &buffer) != 0){
					std::cerr << "Failed doing the CGI pass" << std::endl;
					return false;
				}
				
				break;

			case CLIENT_MAX_BDY:
				location.max_body_size = getMaxRequestBody(noSpaceLine);
				if (location.max_body_size == -1) {
					std::cerr << "Invalid suffix of max body size" << std::endl;
					return false;
				}
				break;

			case BODY_BUFFER:
				location.max_buffer_size = getMaxRequestBody(noSpaceLine);
				if (location.max_body_size == -1) {
					std::cerr << "Invalid suffix of max body size" << std::endl;
					return false;
				}
				break;
			
			case AUTH_LOGIN:
				location.auth_login_page = getInfo(noSpaceLine);
				break;

			case AUTH:
				if (getInfo(noSpaceLine) != "required")
				{
					std::cerr << "The auth variable must be 'required'" << std::endl;
					return false;
				} // If it is not that word we give a error
				location.auth = true;
				break;

			case COMMENT:
			case EMPTY:
				break;

			default:
				return false;
		}
	}

	if (IsLocationOpen == true) // We need to check if the location is closed properly
	{
		std::cerr << "Location is not closed properly" << std::endl;
		return false;
	}

	if (location.name == "cgi" && atIndexFlag == 1) // Checks if exists a autoindex inside a CGI location
		throw std::invalid_argument("ERROR: Can't have autoindex inside a CGI location\n");
	setDefaultLocation(server, location); // We have to set some default values
	server.directories.push_back(location);
	return true;
}

void SetLocation::setDefaultLocation(ServerConfig &server, Directory &location) {
	if (location.max_body_size == 0) {
		if (server.max_body_size != 0)
			location.max_body_size = server.max_body_size;
		else
			location.max_body_size = 1024;
	}

	if (location.max_buffer_size == 0) {
		if (server.max_buffer_size != 0)
			location.max_buffer_size = server.max_buffer_size;
		else
			location.max_buffer_size = 1024 * 1024;
	}

	if (location.path.empty())
		throw std::invalid_argument("Error: Missing path in one or more locations ❌\n");

	if (location.methods.empty()) {
		std::cerr << "No methods, so we will set the GET method ✅" << std::endl;
		location.methods.push_back("GET");
	}

	if (location.root.empty()) {
		if (server.root.empty() == false)
			location.root = server.root;
		else {
			std::cerr << "No root defined. Setting /var/www + path ✅" << std::endl;
			location.root = "/var/www" + location.path;
		}
	}

	if (location.auth_login_page == "/login" && 
		server.auth_login_page != "/login")
	{
		std::cout << "Entrou aqui" << std::endl;
		location.auth_login_page = server.auth_login_page;
		std::cout << "Location nova " << location.auth_login_page << std::endl;
	}
}