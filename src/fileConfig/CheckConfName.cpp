#include "CheckConfName.hpp"
#include <iostream>
#include <fstream>

bool    CheckConfName::checkConfExtension(char* file){
    std::string extension = file;
    size_t      size = extension.size(); // Start at the end of the file

	if (extension.find('.') == std::string::npos){ // Tries to find a '.'
		throw std::invalid_argument("Error: Missing a '.'\n");
		return (false);
	}

    while(extension[size] != '.') // Will find the last '.'
        size--;

    if (extension.substr(size) != ".conf"){ // Whill create a substr and checks for the valid extension
        std::cerr << "Error: Invalid extension!! Use '.conf'" << std::endl;
        return (false);
    }
    return (true);
}

bool    CheckConfName::checkConfOpen(char* file){
    std::ifstream fileFd;
    fileFd.open(file); // Opens the file

    if (!fileFd.is_open()){ // It will check if could open the file
        std::cerr << "Error: Couldn't open the configuration file" << std::endl;
        return (false);
    }
    fileFd.close();
    return (true);
}