#include "httpTcpServer/HttpTcpServerLinux.hpp"

bool isDirectory( const std::string &filePath ) {
	struct stat s;
	if ( stat( filePath.c_str(), &s ) != 0 )
		return ( false );
	return ( S_ISDIR( s.st_mode ) );
}

std::string getFilePath( const std::string &path, const Location &location ) {

	std::string relativePath = path.substr( location.path.length() );
	std::string filePath = joinPath( location.root, relativePath );

	return ( filePath );
}

std::string joinPath( const std::string &base, const std::string &sub ) {
	if ( !base.empty() && ( base[ base.length() - 1 ] == '/' || sub[ 0 ] == '/' ) )
		return ( base + sub );
	return ( base + "/" + sub );
}

std::string getContentType( const std::string &path ) {
	size_t dot = path.find_last_of( '.' );
	if ( dot == std::string::npos )
		return "application/octet-stream"; // generic Binary

	std::string ext = path.substr( dot + 1 );
	if ( ext == "html" || ext == "htm" )
		return "text/html";
	if ( ext == "css" )
		return "text/css";
	if ( ext == "png" )
		return "image/png";
	if ( ext == "jpg" || ext == "jpeg" )
		return "image/jpeg";
	if ( ext == "gif" )
		return "image/gif";
	if ( ext == "txt" )
		return "text/plain";
	if ( ext == "pdf" )
		return "application/pdf";
	return "application/octet-stream";
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

std::string to_str( int n ) {
	std::stringstream ss;
	ss << n;
	return ss.str();
}


// Returns false, if the state is the same. Otherwise we return false
bool containBrackets(std::string &line, bool &state, std::string extraStringToFind)
{
	// If we find the new string
    if (!extraStringToFind.empty())
        return (line.find(extraStringToFind) != std::string::npos);
		
	if (line.find('#') != std::string::npos) // Check if it is a comment
		return true;
		
	int openCount = 0;
	int closeCount = 0;
	for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '{') openCount++;
        else if (line[i] == '}') closeCount++;
    }

	if (openCount > 1 || closeCount > 1 // If we have something like "server {{{{{"
		|| (openCount > 0 && closeCount > 0)) // If we have something like "server }{"
		return false;
		

	if (closeCount > 0)
	{
		if (state == true)
			state = false;
		
		else
		return false; // Return false, if something the state is the same
	}
	
	
	else if (openCount > 0)
	{
		if (state == false)
			state = true;
		
		else
			return false; // Return false, if something the state is the same

	}

	return true;
}


void split_by_string_no_sep(const std::string &s,
                            const std::string &sep,
                            std::string &left,
                            std::string &right)
{
    left = "";
    right = "";

    std::size_t pos = s.find(sep);
    if (pos == std::string::npos)
        return;

    left = s.substr(0, pos);
    right = s.substr(pos + sep.size());
}