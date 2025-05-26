#include "http_tcpServer/http_tcpServerException_linux.hpp"
#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <iostream>
#include <sys/poll.h>
#include <vector>

void http::TcpServer::runServer() {

	int timeOut = 10 * 10 * 1000;

	startServer();
	try {
		startListen();
	} catch (const TcpServerException &e) {
		std::cerr << "Error while starting to listen => " << e.what()
				  << std::endl;
		return;
	}

	std::vector<pollfd> fds;
	struct pollfd listen_fd;
	listen_fd.fd = m_serverSocket;
	listen_fd.events = POLLIN; // any readable data available
	listen_fd.revents = 0;
	fds.push_back(listen_fd);
	try {
		while (true) {
			//
			// poll() waits for events on multiple file descriptors (like
			// sockets), enabling non-blocking I/O in servers.
			int ret = poll(fds.data(), fds.size(), timeOut); // ??
			std::cout << "Entrou no while true do runServer" << std::endl;

			if (ret < 0) {
				std::cerr << "poll() failed" << std::endl;
			} else if (ret == 0) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				shutDownServer();
				return;
			}

			// Checking for new connections
			acceptConnection(fds);
			for (size_t i = 1; i < fds.size(); ++i) {
				int fd = fds[i].fd;

				std::cout << "Entrou no loop do runServer" << std::endl;
				if (fds[i].revents & POLLIN) {
					readRequest(fds, i);
				}
				if (fds[i].revents & POLLOUT) {
					bool shouldClose;
					std::cout << "Entrou no if do runServer" << std::endl;

					validateRequestMethod();
					shouldClose = sendResponse(fds[i]);
					fds[i].events &= ~POLLOUT;
					if (shouldClose) {
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						continue;
					}
					request.headers.clear();
					request.method.clear();
					request.body.clear();
					m_serverMessage.clear();
				}
			}
		}
	} catch (const TcpServerException &e) {
		std::cerr << "Error handling client connection => " << e.what()
				  << std::endl;
	}
	shutDownServer();
}
