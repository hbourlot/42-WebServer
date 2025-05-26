#include "http_tcpServer/http_tcpServerException_linux.hpp"
#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <iostream>
#include <vector>

void http::TcpServer::runServer() {

	int timeOut = 3 * 10 * 1000;

	try {
		startListen();
	} catch (const TcpServerException &e) {
		std::cerr << "Error while starting to listen => " << e.what()
				  << std::endl;
		return;
	}

	fcntl(m_socket, POLLIN,
		  0); // Monitoring listening socket for new connections

	std::vector<pollfd> fds;
	struct pollfd listen_fd;
	listen_fd.fd = m_socket;
	listen_fd.events = POLLIN; // any readable data available
	listen_fd.revents = 0;
	fds.push_back(listen_fd);

	try {
		while (true) {
			int ret = poll(fds.data(), fds.size(), timeOut);

			if (ret < 0) {
				std::cerr << "poll() failed" << std::endl; // Kill the program?
			} else if (ret == 0) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				shutDownServer();
			}

			// Checking for new connections

			if (acceptConnection(fds)) {
				for (size_t i = 1; i < fds.size(); ++i) {
					int fd = fds[i].fd;
					
					if (fds[i].revents & POLLIN) {

						readRequest(fd, fds, i);
						validateRequestMethod();
						sendResponse();
					}
				}

				// Maybe handle POLLOUT for sending response
			}

			// acceptConnection();
			// readRequest();
			// validateRequestMethod();
			// sendResponse();
		}
	} catch (const TcpServerException &e) {
		std::cerr << "Error handling client connection => " << e.what()
				  << std::endl;
	}
	shutDownServer();
	// SOCKET client_socket;
	// m_new_socket = client_socket;
	// acceptConnection(client_socket);
}
