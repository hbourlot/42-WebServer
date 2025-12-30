// #include "webserver.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

void printHttpHeaders( const std::map< std::string, std::string > &headers ) {
	for ( std::map< std::string, std::string >::const_iterator it = headers.begin(); it != headers.end(); ++it ) {
		std::cout << it->first << ": " << it->second << "\n";
	}
}
void printLocation( const Location &location ) {
	// std::cout << << std::endl;
	std::cout << "location.path " << location.path << std::endl;
	std::cout << "location.root " << location.root << std::endl;
	std::cout << "location.redirection " << location.redirection << std::endl;
	std::cout << "location.uploadEnable " << location.uploadEnable << std::endl;
	std::cout << "location.uploadStore " << location.uploadStore << std::endl;
	std::cout << "location.autoIndex " << location.autoIndex << std::endl;
	std::cout << "location.index " << location.index << std::endl;
}
