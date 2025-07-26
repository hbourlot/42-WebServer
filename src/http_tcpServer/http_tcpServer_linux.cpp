#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <iostream>

namespace http {

	TcpServer::TcpServer(Server server)
	    : _serverInfo(server), _serverSocket(), _socketAddress_len(sizeof(sockaddr_in)) {
		// this->startServer();
	}

	TcpServer::~TcpServer() {
		close(_serverSocket);
		// close(_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http