#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>

void http::TcpServer::processClientEvents(std::vector<pollfd> &fds) {

	int fd;
	bool shouldClose;
	sockaddr_in *currentAddress;

	for (size_t i = 1; i < fds.size(); ++i) {

		fd = fds[i].fd;
		currentAddress = &_socketAddress[i];
		if (fds[i].revents & POLLIN)
			readRequest(fds, i);
		if (fds[i].revents & POLLOUT) {
			handleRequest(*currentAddress);
			shouldClose = sendResponse(fds[i]);

			// if (shouldClose)
			// 	exit(0); // ! Never should close???

			fds[i].events &= ~POLLOUT;
			if (shouldClose) {
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				_socketAddress.erase(_socketAddress.begin() + 1);
				--i;
				// continue;
			}
			clearResponse(_request, _serverMessage);
		}
	}
}