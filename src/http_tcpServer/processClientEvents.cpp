#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

bool http::TcpServer::handleCgiResponse(pollfd &socket) {

	std::cout << "[DEBUG] FD: " << socket.fd << " | revents: " << socket.revents
	          << " | events: " << socket.events << std::endl;
	if ((socket.revents & POLLIN) && _cgiFdMap.count(socket.fd)) {
		std::cout << "FD ON HANDLE => " << socket.fd << std::endl;
		Cgi *cgi = _cgiFdMap[socket.fd];
		cgi->readCgiOutput();
		// if (cgi->getStatus() == Cgi::FINISHED) {
		setBodyResponse("200", "OK", cgi->getBody());
		std::cout << cgi->getBody();
		sendResponse(socket);
		_cgiFdMap.erase(socket.fd);
		// }
		return true;
	}
	// std::cout << "RETURNING FALSE\n";
	return false;
}

void http::TcpServer::processClientEvents(std::vector<pollfd> &fds) {

	SocketFD fd;
	bool shouldClose;

	for (size_t i = 1; i < fds.size(); ++i) {

		fd = fds[i].fd;

		if (fds[i].revents & POLLIN) {
			readRequest(fds, i);
		}
		if (handleCgiResponse(fds[i])) {
			continue;
		}
		if (fds[i].revents & POLLOUT) {
			std::cout << "POLLOUT REVENTS\n";
			shouldClose = sendResponse(fds[i]);
			fds[i].events &= ~POLLOUT;
			if (shouldClose) {
				if (_socketAddressMap.count(fds[i].fd)) {
					_socketAddressMap.erase(fds[i].fd);
				}
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				--i;
			}
			clearResponse();
		}
	}
}