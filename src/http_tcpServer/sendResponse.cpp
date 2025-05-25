#include "http_tcpServer_linux.hpp"
#include <cerrno>
#include <sys/socket.h>

int http::TcpServer::sendResponse(pollfd socket) {
	ssize_t bytesSent = send(socket.fd, m_serverMessage.c_str(),
							 m_serverMessage.size(), MSG_NOSIGNAL);
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
	return 0;
}
