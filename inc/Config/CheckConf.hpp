#pragma once
#include <string>
#include <map>
#include <vector>

struct CheckConf {
	static bool checkConfExtension( char *file ); // Check for .conf extension
	static bool checkConfOpen( char *file );      // Check if you can open the file
	static bool
	checkLineFinished( std::string &line ); // Check if you have more information after the ';' or '{' or '}'
	static bool AddHostToPort(std::string host, std::vector<int>, std::map<std::string, std::vector<int> >& serversPort);
};