#include <sstream>

#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "Config/Configs.hpp"

struct Location;
struct File;

namespace http {
	struct Request;
	class Response;
} // namespace http

std::string dateString();
std::string ft_strtrim( const std::string &str );
bool isDirectory( const std::string &filePath );
std::string getFilePath( const std::string &path, const Location &location );
std::string joinPath( const std::string &base, const std::string &sub );
bool containBrackets( std::string &line, bool &state,
                      std::string extraWordToFind ); // Track if the Server on config is open or closed;
void print( const char *src );

template < typename T > std::string ft_to_string( const T &value ) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
};

// !DEBUG
void printHttpHeaders( const std::map< std::string, std::string > &headers );

void printLocation( const Location &location );
bool checkSplitString( const std::string &line, const std::string &sep, bool &isServerOpen );

 File *getMatchFile( const std::string &path,  std::vector< File > &files );
const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations );
std::set<std::string> getAllMethods(ServerConfig server,std::string path);
