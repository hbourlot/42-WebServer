#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstddef>
#include <exception>
#include <ios>
#include <map>
#include <netinet/in.h>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>

// Remove and close all pollfd's with HUP, ERR, or NVAL events
static void removeDeadConnections(std::vector<pollfd> &fds, std::map<SocketFD, sockaddr_in> &socketAddressMap) {

	for (size_t i = 1; i < fds.size(); ++i) {
		if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
			std::cout << "--- Removing CONNECTIONS\n";
			if (socketAddressMap.count(fds[i].fd)) {
				std::cout << "  socketAddressMap => " << fds[i].fd << std::endl;
				socketAddressMap.erase(fds[i].fd);
			}
			std::cout << "  Closing FD => " << fds[i].fd << std::endl;
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			--i;
		}
	}
}

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
			removeDeadConnections(fds, _socketAddressMap);
			processClientEvents(fds);
		}
	} catch (const TcpServerException &e) {
		std::cerr << "Error handling client connection => " << e.what() << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	};
}
