#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <vector>

int http::TcpServer::sendResponse(pollfd socket) {

	// if (m_cgi.data())
	// 	handleCgiResponse(m_cgi[0]);
	// else {

	ssize_t bytesSent = send(socket.fd, _serverMessage.c_str(),
	                         _serverMessage.size(), MSG_NOSIGNAL);
	if (bytesSent < 0) {
		if (errno == EPIPE) {
			log("Client disconnected before response");
		} else {
			log("Error sending response to client");
		}
		return 1;
	} else {
		log("----- Server Response sent to client -----\n\n");
	}
	// }

	return 0;
}
