#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::startListen() {
	int listen_fd;

	// TODO: Need to implement a properly valid max number of padding \
	// TODO: connections
	listen_fd = listen(_serverSocket, 10);

	if (listen_fd < 0) {
		throw TcpServerException("Socket Listen failed");
	}

	std::ostringstream ss; // Output string stream for logging
	ss << "\n*** Listening on ADDRESS: "
	   << inet_ntoa(_socketAddress[0].sin_addr)
	   << " PORT: " << ntohs(_socketAddress[0].sin_port) << " ***\n\n";
	log(ss.str());
}