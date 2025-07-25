#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

void http::TcpServer::closeClientConnection(size_t index) {
	SocketFD fd = _fds[index].fd;

	std::cout << "Closing client FD => " << fd << std::endl;
	if (_clientManager.getClient(fd)->hasCgi()) {
		// and assuming its python CGI
		_clientManager.removeCgiByClientFd(fd);
	}
	close(fd);
	_socketAddressMap.erase(fd);
	_clientManager.removeClient(fd);
	_fds.erase(_fds.begin() + index);
}

void http::TcpServer::processClientEvents() {

	SocketFD fd;
	int shouldCloseSend;
	int shouldCloseRead;
	sockaddr_in *currentAddress;

	for (size_t idx = 1; idx < _fds.size(); ++idx) {
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = _fds[idx].fd;
		if (_fds[idx].revents & POLLIN) {
			shouldCloseRead = readSocket(idx);
		}
		if (_fds[idx].revents & POLLOUT) {
			shouldCloseSend = sendResponse(_fds[idx]);
			if (shouldCloseSend != 2)
				_fds[idx].events &= ~POLLOUT;
			// clearResponse();
		}
		if (shouldCloseRead || shouldCloseSend) {
			closeClientConnection(idx);
			continue;
		}
	}
}