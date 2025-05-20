#include "http_tcpServer_linux.hpp"

void http::TcpServer::runServer() {

	try {
		startListen();
	} catch (const TcpServerException &e) {
		std::cerr << "Error while starting to listen => " << e.what()
				  << std::endl;
		return;
	}

	while (true) {
		try {
			acceptConnection(m_new_socket);
			readRequest();
			validateRequestMethod();
			sendResponse();

		} catch (const TcpServerException &e) {
			std::cerr << "Error handling client connection => " << e.what()
					  << std::endl;
		}
	}

	shutDownServer();
	// SOCKET client_socket;
	// m_new_socket = client_socket;
	// acceptConnection(client_socket);
}
