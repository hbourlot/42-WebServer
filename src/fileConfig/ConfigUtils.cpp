#include "Config/ConfigUtils.hpp"
#include "utils.hpp"

struct Directory;

std::string removeSpace(std::string& line) {
    if (line.empty()) return "";

    const std::string whitespace = " \t\r\n\f\v";
    
    // 1. Achar o primeiro caracter que não é espaço
    size_t start = line.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";

    // 2. Achar o último caracter que não é espaço
    size_t end = line.find_last_not_of(whitespace);

    // 3. RETORNAR EXATAMENTE O QUE ESTÁ NO MEIO
    std::string result = line.substr(start, end - start + 1);
    
    return result;
}

int getMaxRequestBody( std::string& value, size_t& result ) {
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
		resultStr.erase(resultStr.size() - 1);
	}

	if (isDigits(resultStr) == false)
	{
		std::cerr << "Invalid max size" << std::endl;
		return -1;
	}

	char* end;
	long num = std::strtol( resultStr.c_str(), &end, 10 );
	if ( num < 0 ) {
		throw std::runtime_error( "Invalid size number for max size" );
	}
	result = static_cast< size_t >( num ) * multiplier;
	return 1;
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

	if (ss.fail() == true || ss.eof() == false){
		return false;
	}
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