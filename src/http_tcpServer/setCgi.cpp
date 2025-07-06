#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setCgi(SocketFD fd, ICgi *cgi) {
	if (fd >= 1 && cgi) {

		if (this->_cgiFdMap.find(fd) != _cgiFdMap.end()) {
			std::cerr << "Error => Map slot isn't empty!" << std::endl;
		} else {
			this->_cgiFdMap[fd] = cgi;
		}
	}
}