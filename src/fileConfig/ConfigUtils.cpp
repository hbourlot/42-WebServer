#include "Config/ConfigUtils.hpp"

struct Directory;

std::string removeSpace( std::string& line ) {
	int i = 0;

	for ( i = 0; line[ i ] && (line[ i ] == ' ' || line[ i ] == '\t');
	      i++ ) { // Runs all spaces and return the string without the first spaces
		continue;
	}

	return line.substr( i );
}

int getMaxRequestBody( std::string& value ) {
	std::string resultStr = getInfo( value );

	if ( resultStr.empty() == false && resultStr[ resultStr.size() - 1 ] == ';' )
		resultStr.erase( value.size() - 1 );
	// Lets parse the string
	size_t multiplier = 1;
	char last = resultStr[ resultStr.size() - 1 ];
	if ( std::isalpha( last ) ) {
		if ( last == 'K' )
			multiplier = 1024;
		else if ( last == 'M' )
			multiplier = 1024 * 1024;
		else
			return -1;
	}

	char* end;
	long num = std::strtol( resultStr.c_str(), &end, 10 );
	if ( num < 0 ) {
		throw std::runtime_error( "invalid size number" );
	}
	return static_cast< size_t >( num ) * multiplier;
}

std::string getInfo( std::string& noSpaceLine ) {
	int i;

	for ( i = noSpaceLine.find( ' ' ); noSpaceLine[ i ] == ' ';
	      i++ ) // This will remove extra spaces after attribute ex: "port 8080"
		continue;

	if ( noSpaceLine.find( ';' ) == std::string::npos ) // If don't find the ';' throw an error
		throw std::invalid_argument( "Error: Invalid end of line, missing ';' at the end\n" );

	return noSpaceLine.substr( i, noSpaceLine.find( ';' ) - i ); // '- i' I have to discard the 'i' size
}

Directory& findPath( ServerConfig server, std::string path ) {
	std::vector< Directory >::iterator itb = server.directories.begin();
	std::vector< Directory >::iterator ite = server.directories.end();

	while ( itb != ite ) {
		if ( path == itb->path ) {
			return *itb;
		}
		itb++;
	}
	return *ite;
}

bool stringToSizeT(const std::string& str, size_t& result){
	std::stringstream ss(str);
	ss >> result;

	if (ss.fail() == true || ss.eof() == false)
		return false;
	return true;
}

bool stringIsAlpha(const std::string& str){
	if (str.empty() )
		return true;

	for (size_t i = 0; str.size() > i; i++)
	{
		if (std::isalpha(str[i]) == false)
			return false;
	}

	return true;
}