// #include "webserver.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

void printHttpHeaders(const httpRequest &request)
{
	std::map<std::string, std::string>::const_iterator it;
	for (it = request.headers.begin(); it != request.headers.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << "\n";
	}
}

void printLocation(const Location &location)
{
	// std::cout << << std::endl;
	std::cout << "location.path " << location.path << std::endl;
	std::cout << "location.root " << location.root << std::endl;
	std::cout << "location.redirection " << location.redirection << std::endl;
	std::cout << "location.uploadEnable " << location.uploadEnable << std::endl;
	std::cout << "location.uploadStore " << location.uploadStore << std::endl;
	std::cout << "location.autoIndex " << location.autoIndex << std::endl;
}
