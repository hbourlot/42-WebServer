#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

void http::TcpServer::readRequest(std::vector<pollfd> &fds, int i) {

	pollfd *socket;
	char buffer[BUFFER_SIZE + 1] = {0};
	std::string requestContent;

	socket = &fds[i];
	while ((_bytesReceived = read(socket->fd, buffer, BUFFER_SIZE)) > 0)
		requestContent.append(buffer, _bytesReceived);

	if (_bytesReceived < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {

			std::cerr << "Error: read()\n";
			close(socket->fd);
			fds.erase(fds.begin() + i);
			return;
		}
	}
	// if (!requestContent.empty()) {
	// 	if (_socketAddressMap.count(socket->fd)) {
	// 		parseRequest(_request, requestContent, _serverInfo);
	// 		handleRequest(*socket, fds, _socketAddressMap[socket->fd]);

	// 		// fds[i].events |= POLLOUT;
	// 	} else {
	// 		std::cerr << "[ERROR] FD " << socket->fd
	// 		          << " not found in _socketAddressMap\n";
	// 	}
	// }
	if (!requestContent.empty()) {

		parseRequest(_request, requestContent, _serverInfo);
		handleRequest(*socket, fds, _socketAddressMap[socket->fd]);
		// Set event POLLOUT
		// fds[i].events |= POLLOUT;
	}
}
