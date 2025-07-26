#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

static void logAcceptError(const sockaddr_in &socketAddress) {
	std::ostringstream ss;
	ss << "Server failed to accept incoming connection from =>\n"
	   << "[ADDRESS: " << inet_ntoa(socketAddress.sin_addr) << "]\n"
	   << "[PORT: " << ntohs(socketAddress.sin_port) << "]\n";
	std::cerr << ss.str();
}

void http::TcpServer::acceptConnection() {

	SocketFD acceptSocket;
	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	// Checks the if theres readable data available (event)
	while (_fds[0].revents & POLLIN) {
		acceptSocket = accept(_serverSocket, (struct sockaddr *)&socketAddress, &_socketAddress_len);
		if (acceptSocket < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Means no more connections to accept
				break;
			}
			logAcceptError(socketAddress);
			return;
		} else {

			// Set client socket to non-blocking
			fcntl(acceptSocket, F_SETFL, fcntl(acceptSocket, F_GETFL, 0) | O_NONBLOCK);

			client_pollfd.fd = acceptSocket;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;

			_fds.push_back(client_pollfd);
			_socketAddressMap[acceptSocket] = socketAddress;

			_clientManager.addClient(acceptSocket, socketAddress, _fds, _serverInfo);

			std::cout << "----- Connection Accepted 🟩\n\n";
		}
	}
}
