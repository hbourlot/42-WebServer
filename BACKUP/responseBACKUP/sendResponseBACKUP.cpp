#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <vector>

int http::TcpServer::sendResponse(SocketFD fd, std::string contentToSend) {

	std::cout << "OVER HERE\n";
	ssize_t bytesSent = send(fd, contentToSend.c_str(), contentToSend.size(), MSG_NOSIGNAL);
	std::cout << "OVER HERE!@#\n";

	if (bytesSent < 0) {
		if (errno == EPIPE) {
			log("Client disconnected before response");
		} else {
			log("Error sending response to client");
		}
		return 1;
	} else {
		log("----- Server Response sent to client 1-----\n\n");
	}
	return 0;
}
