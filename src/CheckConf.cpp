#include "CheckConf.hpp"
#include <iostream>
#include <fstream>

bool    CheckConf::checkConfExtension(char* file){
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

bool    CheckConf::checkConfOpen(char* file){
    std::ifstream fileFd;
    fileFd.open(file); // Opens the file

    if (!fileFd.is_open()){ // It will check if could open the file
        std::cerr << "Error: Couldn't open the configuration file" << std::endl;
        return (false);
    }
    fileFd.close();
    return (true);
}

bool	CheckConf::checkLineFinished(std::string& line){
	if (line[0] == '#') // Skips the comments
		return true;

	size_t	lineSize = line.size(); // Gets the line size

	size_t	end = line.find_first_of(';'); // Gets the position of ';'
	if (end != std::string::npos){
		if (end + 1 != lineSize){
				// std::cout << "Line: " << line << " | Comma: " << end << " | Line size: " << lineSize << std::endl;

			std::cerr << "Its something wrong with 1: " << line << end << lineSize << std::endl;
			return false;
		}
	}
	
	size_t	openBrackets = line.find_first_of('{'); // Get the position of '{'
	if (openBrackets != std::string::npos){
		if (openBrackets + 1 != lineSize){
				std::cout << "Line: " << line << " | open bracket: " << openBrackets << " | Comma: " << end << " | Line size: " << lineSize << std::endl;

			std::cerr << "Its something wrong with 2: " << line << std::endl;
			return false;
		}
	}

	size_t	closeBrackets = line.find_first_of('}'); // Get the position of '}'
	if (closeBrackets != std::string::npos){
		if (closeBrackets + 1!= lineSize){

			std::cerr << "Its something wrong with 3: " << line << std::endl;
			return false;
		}
	}

	// std::cout << "Line: " << line << " | open bracket: " << openBrackets << " | close brackets: " << closeBrackets << " | Comma: " << end << " | Line size: " << lineSize << std::endl;
	return true;
}