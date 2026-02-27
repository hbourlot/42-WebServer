#include "Config/CheckConf.hpp"
#include <fstream>
#include <iostream>

bool CheckConf::checkConfExtension( char *file ) {
	std::string extension = file;
	size_t size = extension.size(); // Start at the end of the file
	if ( extension.find( '.' ) == std::string::npos ) { // Tries to find a '.'
		throw std::invalid_argument( "Error: Missing a '.'\n" );
		return ( false );
	}

	while ( extension[ size ] != '.' ) // Will find the last '.'
		size--;

	if ( extension.substr( size ) != ".conf" ) { // Whill create a substr and checks for the valid extension
		std::cerr << "Error: Invalid extension!! Use '.conf'" << std::endl;
		return ( false );
	}
	return ( true );
}

bool CheckConf::checkConfOpen( char *file ) {
	std::ifstream fileFd;
	fileFd.open( file ); // Opens the file

	if ( !fileFd.is_open() ) { // It will check if could open the file
		std::cerr << "Error: Couldn't open the configuration file" << std::endl;
		return ( false );
	}
	fileFd.close();
	return ( true );
}

bool CheckConf::checkLineFinished( std::string &line ) {
    if (line.empty() || line[0] == '#') return true;

    size_t actualSize = line.size(); // Ex: 15 para "host 127.0.0.1;"

    size_t semiPos = line.find(';');
    if (semiPos != std::string::npos) {
        if (semiPos != (actualSize - 1)) {
             std::cerr << "Error: Trash after ';'" << std::endl;
             return false;
        }
        return true; 
    }

    size_t openPos = line.find('{');
    if (openPos != std::string::npos) {
        if (openPos != (actualSize - 1)) {
             std::cerr << "Error: Trash after '{'" << std::endl;
             return false;
        }
        return true;
    }

	size_t closeBrackets = line.find( '}' ); // Get the position of '}'
	if ( closeBrackets != std::string::npos ) {
        if (closeBrackets != (actualSize - 1)) {
             std::cerr << "Error: Treash after '{'" << std::endl;
             return false;
        }
        return true;
	}
    return true;
}