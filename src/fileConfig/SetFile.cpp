#include "Config/SetFile.hpp"
#include "Config/SetLocations.hpp"
#include <utils.hpp>

#define METHODS 7
#define ROOT 8
#define REDIRECT 9
#define CGI_EXTENSION 10
#define CGI_PATH 11
#define UPLOAD_ENABLE 12
#define UPLOAD_STORE 13
#define AUTOINDEX 14
#define INDEX 15
#define CGIPASS 16

File::File() {
	// String are automatically initialized;
}

std::string fileExtension(const std::string &line)
{
    size_t start = line.find("*."); // Start where we have to find the path
    if (start == std::string::npos) // If doesn't find any start returns null
        return "";

    size_t end = start;
    while (end < line.size() && 
           !std::isspace(line[end]) &&
           line[end] != '{') // We count the end of the path until we find a 'space' or '{', for cases like "location /cgi bin/hello"
    {
        end++;
    }

    return line.substr(start, end - start);
}

int getTypeFile( std::string &trimedLine ) { // Function to check the information to set
	if ( trimedLine == "methods" )
		return METHODS;
	if ( trimedLine == "root" )
		return ROOT;
	if ( trimedLine == "index" )
		return INDEX;
	if ( trimedLine == "cgi_pass")
		return CGIPASS;
	return 100;
}

bool SetFile::setFileConfig( std::ifstream &confFd, std::string line, ServerConfig &server ) {
	std::string noSpaceLine; // Gets the string without the initial spaces
	std::string trimedLine;  // Stores the atribute of the Location
	std::string emptyString;
	File file;
	bool IsFileOpen = false;

	file.extension = fileExtension( line ); // Gets the file extension

	int atIndexFlag = 0; // Setup a flag for autoIndex, to check for CGI
	if (containBrackets(line, IsFileOpen, emptyString) == false)
	{
		return false; 
	}	
	while ( std::getline( confFd, line ) ) {
		noSpaceLine = removeSpace( line );

		if ( !CheckConf::checkLineFinished( noSpaceLine ) ) // Checks if have more information after the limitter
			throw std::invalid_argument( "Error: Extra words after End of Line\n" );

		trimedLine = noSpaceLine.substr( 0, noSpaceLine.find( ' ' ) );
		
		if (line.find("location") == std::string::npos)
		{
			if (containBrackets(line, IsFileOpen, emptyString) == false)
			{
				return false;
			}
		}
		
		else
		{
			if (checkSplitString(line, "location", IsFileOpen) == false)
			{
				return false;
			}
		}
		
		if ( trimedLine[ 0 ] == '}' )
			break;

		switch ( getTypeFile( trimedLine ) ) {
		case METHODS:
			SetLocation::getMethods( noSpaceLine, file.methods );
			break;
		case ROOT:
			file.root = getInfo( noSpaceLine );
			break;
		case INDEX:
			file.index = getInfo( noSpaceLine );
			break;
		case CGIPASS:
			file.cgi_pass = getInfo( noSpaceLine );
			break;

		default:
			break;
		}
	}
	server.files.push_back( file );
	return true;
}

void SetFile::setDefaultFile( File &file ) {
	if ( file.extension.empty() )
		throw std::invalid_argument( "Error: Missing path in one or more locations ❌\n" );

	if ( file.methods.empty() ) {
		std::cout << "No methods, so we will set the GET method ✅" << std::endl;
		file.methods.push_back( "GET" );
	}

}