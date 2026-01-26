#include <sstream>

#include "httpTcpServer/HttpTcpServerLinux.hpp"

std::string http::Request::GetFileName() {
	std::string::size_type pos = path.rfind( '/' );
	if ( pos == std::string::npos )
		return path;
	return path.substr( pos + 1 );
}

bool isDirectory( const std::string &filePath ) {
	struct stat s;
	if ( stat( filePath.c_str(), &s ) != 0 )
		return ( false );
	return ( S_ISDIR( s.st_mode ) );
}

std::string getFilePath( const std::string &path, const RouteContext &ctx ) {
	std::string relativePath = path.substr( ctx.path.length() );
	std::string filePath = joinPath( ctx.root, relativePath );
	return ( filePath );
}

std::string joinPath( const std::string &base, const std::string &sub ) {
	if ( !base.empty() && ( base[ base.length() - 1 ] == '/' || sub[ 0 ] == '/' ) )
		return ( base + sub );
	return ( base + "/" + sub );
}

std::string ft_strtrim( const std::string &str ) {
	unsigned int start = 0;
	unsigned int end = str.length();

	while ( start < str.length() && std::isspace( str[ start ] ) )
		start++;
	while ( end > start && std::isspace( str[ end - 1 ] ) )
		end--;
	return ( str.substr( start, end - start ) );
}

std::string dateString() {
	time_t timestamp;
	time( &timestamp );
	std::string date = ctime( &timestamp );
	if ( !date.empty() && date[ date.length() - 1 ] == '\n' )
		date.erase( date.length() - 1 );
	return ( date );
}

// Returns false, if the state is the same. Otherwise we return false
bool containBrackets( std::string &line, bool &state, std::string extraStringToFind ) {
	// If we find the new string
	if ( !extraStringToFind.empty() )
		return ( line.find( extraStringToFind ) != std::string::npos );

	if ( line.find( '#' ) != std::string::npos ) // Check if it is a comment
		return true;

	int openCount = 0;
	int closeCount = 0;
	for ( size_t i = 0; i < line.size(); ++i ) {
		if ( line[ i ] == '{' )
			openCount++;
		else if ( line[ i ] == '}' )
			closeCount++;
	}

	if ( openCount > 1 || closeCount > 1          // If we have something like "server {{{{{"
	     || ( openCount > 0 && closeCount > 0 ) ) // If we have something like "server }{"
	{
		std::cerr << "More than one bracket in the same line" << std::endl;
		return false;
	}

	if ( closeCount > 0 ) {
		if ( state == true )
			state = false;

		else {
			std::cerr << "In configuration, is trying to close when its already closed" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	else if ( openCount > 0 ) {
		if ( state == false )
			state = true;

		else {
			std::cerr << "In configuration, is trying to open when its already open" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	return true;
}

// This method is to check especific cases like " {{{ location / }}}"
// This way we split the string by the location and see if we open him after
// Only for location cases
bool checkSplitString( const std::string &line, const std::string &sep, bool &isServerOpen ) {
	std::string left = "";
	std::string right = "";
	std::string empty = "";

	std::size_t pos = line.find( sep );
	if ( pos == std::string::npos )
		return true;

	left = line.substr( 0, pos );
	right = line.substr( pos + sep.size() );

	if ( isServerOpen == true ) // Since its already open, checks only the left
	{
		if ( containBrackets( left, isServerOpen, empty ) == false )
			return false;
		return true;
	}

	if ( isServerOpen == false ) // Checks the right side if it is open in the same line " location /upload {"
	{
		if ( containBrackets( right, isServerOpen, empty ) == false )
			return false;
	}

	return true;
}

void print( const char *src ) {

	std::string a( src );
	std::cout << a << std::endl;
};

static std::string GetExtension( const std::string &path ) {
	size_t slashPos = path.find_last_of( '/' );
	size_t dotPos = path.find_last_of( '.' );

	if ( dotPos == std::string::npos )
		return "";

	if ( slashPos != std::string::npos && dotPos < slashPos )
		return "";

	return path.substr( dotPos );
}

const File *getMatchFile( const std::string &path, const std::vector< File > &files ) {
	std::string ext = GetExtension( path ); // ".txt", ".py", etc.
	if ( ext.empty() )
		return NULL;

	for ( size_t i = 0; i < files.size(); ++i ) {
		if ( files[ i ].extension.find( ext ) != std::string::npos ) {
			return &files[ i ];
		}
	}
	return NULL;
}

const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations ) {

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for ( size_t i = 0; i < locations.size(); ++i ) {

		const std::string &locPath = locations[ i ].path;
		if ( path.find( locPath ) != std::string::npos && locPath.size() > matchLength ) {
			matchedLocation = &locations[ i ];
			matchLength = locPath.size();
		}
	}
	return ( matchedLocation );
}

RouteContext makeContext( const MatchResult &match, const ServerConfig &server, http::Request &request,
                          VALIDATION_STATUS status ) {
	RouteContext ctx;

	// 1️⃣ CGI FILE has priority?
	if ( status == VALID_IS_CGI && match.file ) {
		const File &file = *match.file;
		ctx.methods = file.methods;
		ctx.path = "";
		ctx.root = file.root;
		ctx.index = file.index;
		ctx.autoIndex = false;
		ctx.max_body_size = server.max_body_size;
		ctx.cgi_pass = file.cgi_pass;
		ctx.redirection = "";
		ctx.uploadEnable = false;
		ctx.uploadStore = "";
	}
	// 2️⃣ Location normal
	else if ( match.location ) {
		const Location &loc = *match.location;
		ctx.methods = loc.methods;
		ctx.path = loc.path;
		ctx.root = loc.root;
		ctx.index = loc.index;
		ctx.autoIndex = loc.autoIndex;
		std::cout << "loc.max_body_size: " << loc.max_body_size << std::endl;
		ctx.max_body_size = loc.max_body_size;
		// ctx.max_body_size = 100; //! Brute code for the script that works
		if ( !loc.cgi_pass.empty() )
			ctx.cgi_pass = loc.cgi_pass;
		else
			ctx.cgi_pass = joinPath( loc.root, request.GetFileName() );
		ctx.redirection = loc.redirection;
		ctx.uploadEnable = loc.uploadEnable;
		ctx.uploadStore = loc.uploadStore;
	}
	// 3️⃣ File static
	else if ( match.file ) {
		const File &file = *match.file;
		ctx.methods = file.methods;
		ctx.path = "";
		ctx.root = file.root;
		ctx.index = file.index;
		ctx.autoIndex = false;
		ctx.max_body_size = server.max_body_size;
		ctx.cgi_pass = file.cgi_pass;
		ctx.redirection = "";
		ctx.uploadEnable = false;
		ctx.uploadStore = "";
	}

	ctx.isCgi = ( status == VALID_IS_CGI );
	ctx.isRedirect = ( status == VALID_REDIRECT_REQUIRED );

	return ctx;
}

std::string createUploadBody() {
	std::string html;

	html += "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n<title>Error Occurred</title>\n";
	html += "<style>\n";
	html +=
	    "body { font-family: Arial, sans-serif; text-align: center; padding-top: 10%; background-color: #f9f9f9; }\n";
	html += ".error-box { display: inline-block; border-radius: 10px; padding: 2em 3em; background: #fff; box-shadow: "
	        "0 2px 8px rgba(0,0,0,0.1); }\n";
	html += "h1 { margin-bottom: 0.5em; color: #5ad94f; }\n";
	html += "p { color: #555; margin-bottom: 1.5em; }\n";
	html += "a.button { text-decoration: none; color: white; background: #007bff; padding: 0.7em 1.5em; border-radius: "
	        "5px; font-weight: bold; }\n";
	html += "a.button:hover { background: #0056b3; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<div class=\"error-box\">";
	html += "<h1>Upload OK</h1>";
	html += "<a href=\"/pages/Services/Services.html\" class=\"button\">Back</a>\n";
	html += "</div>\n";
	html += "</body>\n</html>\n";

	return html;
}

// std::string getFileExtension( const std::string &path ) {
// 	size_t slashPos = path.find_last_of( '/' );
// 	size_t dotPos = path.find_last_of( '.' );

// 	if ( dotPos == std::string::npos || ( slashPos != std::string::npos && dotPos < slashPos ) )
// 		return "";
// 	return path.substr( dotPos );
// }

// std::vector< std::string > splitLocations( const std::string &path ) {
// 	std::vector< std::string > result;
// 	std::string current;
// 	size_t i = 0;

// 	// assumir que path começa com '/'
// 	if ( path.empty() || path[ 0 ] != '/' )
// 		return result;

// 	i = 1; // saltar o '/'

// 	while ( i < path.length() ) {
// 		size_t next = path.find( '/', i );

// 		if ( next == std::string::npos ) {
// 			current += path.substr( i );
// 			result.push_back( "/" + current );
// 			break;
// 		} else {
// 			current += path.substr( i, next - i );
// 			result.push_back( "/" + current );
// 			current += "/";
// 			i = next + 1;
// 		}
// 	}

// 	return result;
// }

// void convertSetToVector( std::set< std::string > &setCtr, std::vector< std::string > &vecCtr ) {
// 	for ( std::set< std::string >::iterator i = setCtr.begin(); i != setCtr.end(); i++ )
// 		vecCtr.push_back( *i );
// }

// /// @brief Search in every directory and gets every method (ex. /directory/alias/test)
// /// @param path Full directory
// /// @return vector with all getted methods
// std::vector< std::string > getAllMethods( ServerConfig server, std::string path ) {
// 	std::vector< std::string > mergedMethods;
// 	std::vector< std::string > splittedLocations;
// 	std::set< std::string > noDupMethods;
// 	splittedLocations = splitLocations( path );
// 	for ( size_t i = 0; i < splittedLocations.size(); i++ ) {
// 		Location *curLocation = server.GetLocationByPath( splittedLocations[ i ] );

// 		if ( curLocation != NULL ) {
// 			for ( size_t x = 0; x < curLocation->methods.size(); x++ )
// 				noDupMethods.insert( curLocation->methods[ x ] );

// 			if ( noDupMethods.size() >= 3 ) {
// 				convertSetToVector( noDupMethods, mergedMethods );
// 				return mergedMethods;
// 			}
// 		}
// 	}

// 	File *curFile = server.GetFileByExtension( "*" + getFileExtension( path ) );
// 	if ( curFile != NULL ) {
// 		for ( size_t x = 0; x < curFile->methods.size(); x++ ) {
// 			noDupMethods.insert( curFile->methods[ x ] );
// 		}
// 	}

// 	// Send the "SET" info to the "VECTOR"
// 	convertSetToVector( noDupMethods, mergedMethods );
// 	return mergedMethods;
// }

// std::string getContentType( const std::string &path ) {
// 	size_t dot = path.find_last_of( '.' );
// 	if ( dot == std::string::npos )
// 		return "application/octet-stream"; // generic Binary

// 	std::string ext = path.substr( dot + 1 );
// 	if ( ext == "html" || ext == "htm" )
// 		return "text/html";
// 	if ( ext == "css" )
// 		return "text/css";
// 	if ( ext == "png" )
// 		return "image/png";
// 	if ( ext == "jpg" || ext == "jpeg" )
// 		return "image/jpeg";
// 	if ( ext == "gif" )
// 		return "image/gif";
// 	if ( ext == "txt" )
// 		return "text/plain";
// 	if ( ext == "pdf" )
// 		return "application/pdf";
// 	return "application/octet-stream";
// }
