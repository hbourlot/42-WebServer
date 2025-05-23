#pragma once
#include <string>

struct CheckConf{
    static bool    	checkConfExtension(char* file); // Check for .conf extension
    static bool    	checkConfOpen(char* file); // Check if you can open the file
	static bool		checkLineFinished(std::string& line); // Check if you have more information after the ';' or '{' or '}'
};