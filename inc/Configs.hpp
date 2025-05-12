#pragma once
#include <vector>
#include <string>
#include <map>

struct Route{
	std::string					methods;
    std::string                 root;
};

struct Server{
	std::string					host; // Stores the host IP
	std::string					port; // Stores the port to listen
	std::string					serverName; // Stores the name server
	std::map<int, std::string>	errorPage; // Stores the error pages
	int							maxRequest; // Stores the maximum requests that the client can do
	std::vector<Route>			routes; // Stores the routes of the HTML pages
};

struct Configs{
	std::vector<Server>	servers; // Will store the servers
};