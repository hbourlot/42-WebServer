#include "SetLocations.hpp"

#define METHODS 7
#define ROOT 8

std::string locationPath(std::string& line){
	int i;
	
	for (i = 0; line[i] && line[i] == ' '; i++) //Skip the spaces
		continue;
	
	int j;
	for (j = 0; line[j] && line[j] != ' ' && line[j] != '{'; j++) // Gets the path size
		continue;

	return line.substr(i,j + 1); // Return the path
}

void	getMethods(std::string noSpaceLine, std::vector<std::string>& methods){ // Function to get the Location methods
	std::istringstream iss(noSpaceLine);
    std::string method;
	
	iss >> method; // Skip the method word
	while (iss >> method){ // Saves the new method
		if (!method.empty() && method[method.size() - 1] == ';')
    		method.erase(method.size() - 1);
		methods.push_back(method); // Send it for the method variable
	}
}

int		getTypeLocation(std::string& trimedLine){ // Function to check the information to set
	if (trimedLine == "methods")
		return METHODS;
	if (trimedLine == "root")
		return ROOT;

	return 10;
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
			default:
				break;

		}
	}
	server.locations.push_back(location);
	return true;
}