#pragma once
#include <iostream>

struct Location;

namespace http {
	struct Request;
} // namespace http

std::string dateString();
std::string to_str( int n );
std::string ft_strtrim( const std::string &str );
bool isDirectory( const std::string &filePath );
std::string getFilePath( const std::string &path, const Location &location );
std::string joinPath( const std::string &base, const std::string &sub );
bool containBrackets (std::string &line, bool &state, std::string extraWordToFind); // Track if the Server on config is open or closed;

// !DEBUG
void printHttpHeaders( const http::Request &request );

void printLocation( const Location &location );
