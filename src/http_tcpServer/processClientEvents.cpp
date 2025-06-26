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
		setBodyResponse(HTTP_OK, cgi->getBody());
		std::cout << cgi->getBody();
		sendResponse(socket);
		_cgiFdMap.erase(socket.fd);
		// }
		return true;
	}
	// std::cout << "RETURNING FALSE\n";
	return false;
}

void http::TcpServer::closeClient(std::vector<pollfd> &fds, size_t &i) {
	int fd = fds[i].fd;
	close(fd);
	fds.erase(fds.begin() + i);
	if (_socketAddressMap.count(fds[i].fd)) {
		_socketAddressMap.erase(fds[i].fd);
	}
	--i;
}

void http::TcpServer::processClientEvents(std::vector<pollfd> &fds) {

	SocketFD fd;
	bool shouldCloseSend;
	bool shouldCloseRead;
	sockaddr_in *currentAddress;

	for (size_t i = 1; i < fds.size(); ++i) {
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = fds[i].fd;

		if (fds[i].revents & POLLIN) {
			shouldCloseRead = readRequest(fds, i);
		}
		// if (handleCgiResponse(fds[i])) {
		// 	continue;
		// }
		// std::cout << "Here " << std::endl;
		if (fds[i].revents & POLLOUT) {
			// std::cout << "Here " << __func__ << std::endl;

			
			shouldCloseSend = sendResponse(fds[i]);
			fds[i].events &= ~POLLOUT;
			clearResponse();
		}
		if (shouldCloseRead || shouldCloseSend) {
			closeClient(fds, i);
			continue;
		}
	}
}