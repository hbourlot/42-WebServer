#include "http_tcpServer_linux.hpp"

void http::TcpServer::startListen() {
	int listen_fd;

	listen_fd = listen(m_socket, 20);

	if (listen_fd < 0) {
		throw TcpServerException("Socket Listen failed");
	}

	std::ostringstream ss; // Output string stream for logging
	ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr)
	   << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
	log(ss.str());
}