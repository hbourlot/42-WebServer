#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

static void setSocketAddr(sockaddr_in &socketAddress, int domain, int s_addr,
                          int _port) {
	socketAddress.sin_family = domain;
	socketAddress.sin_addr.s_addr =
	    s_addr; // can replace this with a specific IP address if needed
	socketAddress.sin_port =
	    htons(_port); // Converts 16-bit integer in host byte order
}

namespace http {

	int TcpServer::startServer() {

		// Creates a server socket (IPv4, TCP, 0) (domain, type, protocol);
		_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (_serverSocket < 0) {
			throw TcpServerException("Cannot create socket");
			return -1;
		}

		// Set listening socket to non-blocking mode
		fcntl(_serverSocket, F_SETFL,
		      fcntl(_serverSocket, F_GETFL, 0) | O_NONBLOCK);

		// For inactivate the time wait from OS that block bind again
		int opt = 1;
		if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
		               sizeof(opt)) < 0) {
			log("setsockopt failed");
			close(_serverSocket);
			exit(EXIT_FAILURE);
		}
		struct sockaddr_in socketAddress;
		// Set the socket address struct
		setSocketAddr(socketAddress, AF_INET, INADDR_ANY, _port);

		// Associate socket with a specific IP addr and Port number (sockfd,
		// sockaddr *, addrlen)
		if (bind(_serverSocket, (sockaddr *)&socketAddress,
		         _socketAddress_len) < 0) {
			perror("bind");
			throw TcpServerException("Cannot bind socket to address");
			return -1;
		}
		_socketAddress.push_back(socketAddress);
		return 0;
	}
} // namespace http