#include "Config/SetFile.hpp"
#include "Config/SetLocations.hpp"
#include <utils.hpp>


File::File() {
	// String are automatically initialized;
}

bool fileExtension( const std::string &line, std::string &fileExtension ) {
	if ( line[1] != '*' || line[2] != '.' ) {
		std::cerr << "Failed to set the file extension" << std::endl;
		return false;
	}
	size_t start = 3;

	size_t end = start;
	while ( end < line.size() && !std::isspace( line[end] ) &&
			line[end] != '{' ) // We count the end of the path until we find a 'space' or '{', for cases like "location
							   // /cgi bin/hello"
	{
		end++;
	}
	fileExtension = line.substr( start, end - start );
	if ( stringIsAlpha( fileExtension ) == false ) {
		std::cerr << "File extension is not valid" << std::endl;
		return false;
	}
	return true;
}

int getTypeFile( std::string &trimmedLine ) { // Function to check the information to set
	if ( trimmedLine == "methods" )
		return METHODS;
	if ( trimmedLine == "root" )
		return ROOT;
	if ( trimmedLine == "index" )
		return INDEX;
	if ( trimmedLine == "cgi_pass" )
		return CGIPASS;
	if ( trimmedLine.size() == 1 || trimmedLine == "{" || trimmedLine == "}" || trimmedLine == "\n" )
		return EMPTY;
	if ( trimmedLine[0] == '#' )
		return COMMENT;
	return 100;
}

bool SetFile::setFileConfig( std::ifstream &confFd, std::string line, ServerConfig &server ) {
	std::string noSpaceLine; // Gets the string without the initial spaces
	std::string trimmedLine; // Stores the attribute of the Location
	std::string emptyString;
	File file;
	bool IsFileOpen = false;

	if ( fileExtension( line, file.extension ) == false )
		return false;

	file.max_buffer_size = server.max_buffer_size;
	if ( containBrackets( line, IsFileOpen, emptyString ) == false ) {
		return false;
	}
	while ( std::getline( confFd, line ) ) {
		noSpaceLine = removeSpace( line );

		if ( !CheckConf::checkLineFinished( noSpaceLine ) ) // Checks if have more information after the limiter
			throw std::invalid_argument( "Error: Extra words after End of Line\n" );

		trimmedLine = noSpaceLine.substr( 0, noSpaceLine.find( ' ' ) );

		if ( line.find( "location" ) == std::string::npos ) {
			if ( containBrackets( line, IsFileOpen, emptyString ) == false ) {
				return false;
			}
		}

		else {
			if ( checkSplitString( line, "location", IsFileOpen ) == false ) {
				return false;
			}
		}

		if ( trimmedLine[0] == '}' )
			break;

		switch ( getTypeFile( trimmedLine ) ) {
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
			struct stat buffer;

			if ( stat( file.cgi_pass.c_str(), &buffer ) != 0 )
				return false;
			break;

		case EMPTY:
		case COMMENT:
			break;
		default:
			break;
		}
	}
	setDefaultFile( file );
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