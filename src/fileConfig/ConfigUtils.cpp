#include "Config/ConfigUtils.hpp"

std::string removeSpace( std::string &line ) {
	int i = 0;

	for ( i = 0; line[ i ] && line[ i ] == ' ';
	      i++ ) { // Runs all spaces and return the string without the first spaces
		continue;
	}

	return line.substr( i );
}

int getMaxRequestBody(std::string &value) {
	int 		result = 2048; // Default value
	std::string resultStr = getInfo(value);
	
	if (resultStr.empty() == false &&
		resultStr[resultStr.size() - 1] == ';')
		resultStr.erase(value.size() - 1);

	// Lets parse the string
    size_t multiplier = 1;
	char last = value[value.size() - 1];
    if (std::isalpha(last))
    {
        resultStr = value.substr(0, value.size() - 1);

        if (last == 'K')
            multiplier = 1024;
        else if (last == 'M')
            multiplier = 1024 * 1024;
        else
            throw std::runtime_error("invalid size suffix");
    }
	
	
    char* end;
    long num = std::strtol(resultStr.c_str(), &end, 10);
    if (num < 0) {
		throw std::runtime_error("invalid size number");
	}
	
    return static_cast<size_t>(num) * multiplier;

	return result;
}

std::string getInfo( std::string &noSpaceLine ) {
	int i;

	for ( i = noSpaceLine.find( ' ' ); noSpaceLine[ i ] == ' ';
	      i++ ) // This will remove extra spaces after attribute ex: "port      8080"
		continue;

	if ( noSpaceLine.find( ';' ) == std::string::npos ) // If don't find the ';' throw an error
		throw std::invalid_argument( "Error: Invalid end of line, missing ';' at the end\n" );

	return noSpaceLine.substr( i, noSpaceLine.find( ';' ) - i ); // '- i' I have to discard the 'i' size
}

Location &findPath( ServerConfig server, std::string path ) {
	std::vector< Location >::iterator itb = server.locations.begin();
	std::vector< Location >::iterator ite = server.locations.end();

	while ( itb != ite ) {
		if ( path == itb->path ) {
			return *itb;
		}
		itb++;
	}
	return *ite;
}