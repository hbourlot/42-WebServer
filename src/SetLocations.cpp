#include "SetLocations.hpp"

#define METHODS 7
#define ROOT 8
#define REDIRECT 9
#define CGI_EXTENSION 10
#define CGI_PATH 11

std::string locationPath(const std::string& line) {
    size_t start = line.find_first_not_of(' '); // Skips all the spaces
    if (start == std::string::npos)
        return "";

    size_t end = line.find_first_of(" {", start); // Will find the ' {'
    if (end == std::string::npos)
        end = line.length();

    return line.substr(start, end - start);
}


void	getMethods(std::string noSpaceLine, std::vector<std::string>& methods){ // Function to get the Location methods
	std::istringstream iss(noSpaceLine);
    std::string method;
	
	iss >> method; // Skip the method word
	while (iss >> method){ // Saves the new method
		if (!method.empty() && method[method.size() - 1] == ';')
    		method.erase(method.size() - 1);

		if (method != "GET" && method != "POST" && method != "DELETE") // Check if the method is valid
			throw std::invalid_argument("Error: Invalid method!!! Use only (GET/POST/DELETE)");

		methods.push_back(method); // Send it for the method variable
	}
}

int		getTypeLocation(std::string& trimedLine){ // Function to check the information to set
	if (trimedLine == "methods")
		return METHODS;
	if (trimedLine == "root")
		return ROOT;
	if (trimedLine == "redirect")
		return REDIRECT;
	if (trimedLine == "cgi_extension")
		return CGI_EXTENSION;
	if (trimedLine == "cgi_path")
		return CGI_PATH;

	return 100;
}

void	buildCgi(Location& location){
	size_t sizeCgiPath = location.cgi_extension.size(); // Get the size of the cgi paths vector
	size_t sizeCgiExtension = location.cgi_path.size(); // Get the size of the cgi extensions vector

	if (sizeCgiExtension != sizeCgiPath) // Check the size of each containers to see if they match
		throw std::invalid_argument("Error: Not the same number as CGI paths and extensions\n");

	for (int i = 0; sizeCgiExtension > i && sizeCgiPath > i; i++) // Insert the information on the map
		location.cgi.insert(std::make_pair(location.cgi_extension[i], location.cgi_path[i])); // I have to make a pair to accept

	// Check for dup information maybe 
}

int		getCgi(std::string noSpaceLine, Location& location, int cgiInfo){
	std::istringstream iss(noSpaceLine);
    std::string info;
	
	iss >> info;

	int	ready = 0;
	if (!location.cgi_extension.empty() || !location.cgi_path.empty()) // It will check if we already have any pre requisites to build the cgi
		ready = 1;

	while (iss >> info){ // Saves the new info

		if (!info.empty() && info[info.size() - 1] == ';')
    		info.erase(info.size() - 1);
		
		if (cgiInfo == CGI_EXTENSION)
			location.cgi_extension.push_back(info); // Send it for the cgi_extension variable
		else
			location.cgi_path.push_back(info); // Send it for the cgi_path variable
	}

	return ready; // If 0, not enough information | If 1, ready to build the map
}

bool	SetLocation::setLocationConfig(std::ifstream& confFd, std::string line, Server& server){
	std::string noSpaceLine; // Gets the string without the initial spaces	
	std::string trimedLine; // Stores the atribute of the Location
	Location location;

	location.path = locationPath(line); // Sets the Location path

	while (std::getline(confFd, line)){
		noSpaceLine = removeSpace(line);
		trimedLine = noSpaceLine.substr(0, noSpaceLine.find(' '));

		if (trimedLine[0] == '}')
			break;
		
		switch(getTypeLocation(trimedLine)){
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
				if (getCgi(noSpaceLine, location, CGI_EXTENSION) == 1)
					buildCgi(location); // -> Missing function
				break;
			case CGI_PATH:
				if (getCgi(noSpaceLine, location, CGI_PATH) == 1)
					buildCgi(location);
				break;
			default:
				break;

		}
	}
	server.locations.push_back(location);
	return true;
}