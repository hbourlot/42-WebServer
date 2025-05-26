#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <vector>

void http::TcpServer::runLoop(std::vector<pollfd> &fds, int timeOut) {

	try {
		while (true) {
			// poll() waits for events on multiple file descriptors (like
			// sockets), enabling non-blocking I/O in servers.
			int ret = poll(fds.data(), fds.size(), timeOut);

			if (ret < 0) {
				std::cerr << "poll() failed" << std::endl;
				shutDownServer(fds);
			} else if (ret == 0) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				shutDownServer(fds);
				return;
			}

			// Checking for new connections
			acceptConnection(fds);
			for (size_t i = 1; i < fds.size(); ++i) {
				int fd = fds[i].fd;
				std::cout << "INSIDE FOR LOOP\n";
				if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
					std::cout << "EXIT POLLHUP...'\n";
					exit(0);
					continue;
				}

				if (fds[i].revents & POLLIN) {
					std::cout << "POLLIN\n";
					readRequest(fds, i);
				}
				if (fds[i].revents & POLLOUT) {

					bool shouldClose;
					validateRequestMethod();
					shouldClose = sendResponse(fds[i]);

					if (shouldClose) {
						std::cout << "SHOULD CLOSE\n";
						exit(0);
					}
					fds[i].events &= ~POLLOUT;
					if (shouldClose) {
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
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
}
