#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

// bool http::TcpServer::handleCgiResponse(pollfd &socket) {

// 	if ((socket.revents & POLLIN) && _cgiFdMap.count(socket.fd)) {
// 		Cgi *cgi = _cgiFdMap[socket.fd];
// 		if (cgi->getStatus() != Cgi::FINISHED ||
// 			cgi->getStatus() != Cgi::ERROR) {
// 			cgi->readCgiOutput();
// 		}
// 		// if (cgi->getStatus() == Cgi::FINISHED) {
// 		setBodyResponse(HTTP_OK, cgi->getOutputContent());
// 		std::cout << cgi->getOutputContent();
// 		sendResponse(socket);
// 		_cgiFdMap.erase(socket.fd);
// 		// }
// 		return true;
// 	}
// 	// std::cout << "RETURNING FALSE\n";
// 	return false;
// }

void http::TcpServer::closeClientConnection(size_t index) {
	SocketFD fd = _fds[index].fd;

	std::cout << "Closing client FD => " << fd << std::endl;

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
	std::vector<int> toClose;

	for (int idx = _fds.size() -1 ; idx >= 1; --idx) {
		std::cout << "Index: " << idx << std::endl;
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = _fds[idx].fd;
		if (_fds[idx].revents & POLLIN) {
			shouldCloseRead = readSocket(idx);
		}
		if (_fds[idx].revents & POLLOUT) {
			std::cout << "Segfault 1" << std::endl;
			shouldCloseSend = sendResponse(_fds[idx]);
			if (shouldCloseSend != 2)
				_fds[idx].events &= ~POLLOUT;
			// clearResponse();
		}
		if (shouldCloseRead || shouldCloseSend) {
			std::cout << "Vamos fechar com o close client connection" << std::endl;
			// closeClientConnection(idx);
			toClose.push_back(idx);
			// continue;
		}
	}
	
	for (size_t i = 0; i < toClose.size(); ++i) {
		std::cout << "Entrou para fechar um fd" << std::endl;
		closeClientConnection(toClose[i]);
	}
	std::cout << "Saiu do for loop" << std::endl;
}