#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <vector>

int http::TcpServer::sendResponse(pollfd &socket) {
	SocketFD fd = socket.fd;

	Client &client = *_clientManager.getClient(fd);

	if (client.hasCgi()) {

		std::cerr << "ON SENDRESPONSE = " << fd << std::endl;

		std::string buff = client.getCgi()->getOutput();
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
		httpResponse += buff;
		// std::cout << client.getCgi()->getOutput();
		ssize_t bytesSent = send(fd, httpResponse.c_str(), httpResponse.size(), MSG_NOSIGNAL);
		return 2;
	}

	std::string &writeBuffer = client.getWriteBuffer();
	// std::cout << "writeBuffer "<<writeBuffer << "\n";

	if (writeBuffer.empty())
		return 0;

	ssize_t bytesSent = send(fd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);

	if (bytesSent < 0) {
		if (errno == EPIPE)
			log("Client disconnected before response");
		else
			log("Error sending response to client");
		return 1; // cerrar conexión
	}

	writeBuffer.erase(0, bytesSent);

	if (writeBuffer.empty()) {
		log("----- Server Response sent to client -----\n\n");
		return 0;
	}

	// Si todavía quedan datos, esperar siguiente POLLOUT
	return 2; // aún quedan datos por enviar
}
