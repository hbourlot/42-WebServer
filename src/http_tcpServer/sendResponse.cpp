#include "http_tcpServer_linux.hpp"

void http::TcpServer::sendResponse(pollfd socket) {
	ssize_t bytesSent =
		send(socket.fd, m_serverMessage.c_str(), m_serverMessage.size(), 0);
	if (bytesSent < 0) {
		log("Error sending response to client");
	} else {
		log("----- Server Response sent to client -----\n\n");
	}
	// request.body = "";
	// request.headers.clear();
	// m_serverMessage = "";
	// request.method = "";
}
