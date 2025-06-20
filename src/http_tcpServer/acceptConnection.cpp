#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

void http::TcpServer::acceptConnection(std::vector<pollfd> &fds) {

	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	// Checks the if theres readable data available (event)
	while (fds[0].revents & POLLIN) {
		_acceptSocket = accept(_serverSocket, (struct sockaddr *)&socketAddress,
		                       &_socketAddress_len);
		if (_acceptSocket < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Means no more connections to accept
				break;
			}
			std::ostringstream ss;

			ss << "Server failed to accept incoming connection from =>\n"
			      "[ADDRESS: "
			   << inet_ntoa(socketAddress.sin_addr) << "]\n"
			   << "[PORT: " << ntohs(socketAddress.sin_port) << "]\n";
			std::cerr << ss.str();
			return;
		} else {

			// Set client socket to non-blocking
			fcntl(_acceptSocket, F_SETFL,
			      fcntl(_acceptSocket, F_GETFL, 0) | O_NONBLOCK);

			client_pollfd.fd = _acceptSocket;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;

			fds.push_back(client_pollfd);
			_socketAddress.push_back(socketAddress);
			std::cout << "----- Connection Accepted 🟩\n\n";
		}
	}
}
