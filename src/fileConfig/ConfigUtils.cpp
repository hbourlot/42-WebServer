#include "Config/ConfigUtils.hpp"

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

Location&   findPath(Server server, std::string path){
	std::vector<Location>::iterator itb = server.locations.begin();
	std::vector<Location>::iterator ite = server.locations.end();

	while (itb != ite){
		if (path == itb->path){
			return *itb;
		}
		itb++;
	}
	return *ite;
}