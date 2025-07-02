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

