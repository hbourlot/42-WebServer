#pragma once
#include "CheckConf.hpp"
#include "ConfigUtils.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Location
{
	std::string path;                 // location --> /upload <--
	std::vector<std::string> methods; // method POST GET DELETE
	std::string root;
	std::string index;
	std::string redirection; // http://example.com;
	std::vector<std::string> cgi_extension;
	std::vector<std::string> cgi_path;
	std::map<std::string, std::string> cgi;
	bool uploadEnable;
	std::string uploadStore;
	bool autoIndex;
};

struct Server
{
	std::string host;                     // Stores the host IP
	int port;                             // Stores the port to listen
	std::string serverName;               // Stores the name server
	std::map<int, std::string> errorPage; // Stores the error pages
	int maxRequest; // Stores the maximum requests that the client can do
	std::vector<Location> locations; // Stores the routes of the HTML pages
	// std::map<std::string (path),Location>		locations; //! Alternative
	// way???
};

struct Configs
{
	std::vector<Server> servers; // Will store the servers
};
