#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

static void setSocketAddr(sockaddr_in &m_socketAddress, int domain, int s_addr,
						  int m_port) {
	m_socketAddress.sin_family = domain;
	m_socketAddress.sin_addr.s_addr =
		s_addr; // can replace this with a specific IP address if needed
	m_socketAddress.sin_port =
		htons(m_port); // Converts 16-bit integer in host byte order
}

namespace http {

	int TcpServer::startServer() {

		// Creates a server socket (IPv4, TCP, 0) (domain, type, protocol);
		m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_serverSocket < 0) {
			throw TcpServerException("Cannot create socket");
			return -1;
		}

		// Set listening socket to non-blocking mode
		fcntl(m_serverSocket, F_SETFL,
			  fcntl(m_serverSocket, F_GETFL, 0) | O_NONBLOCK);

		// For inactivate the time wait from OS that block bind again
		int opt = 1;
		if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
					   sizeof(opt)) < 0) {
			log("setsockopt failed");
			close(m_serverSocket);
			exit(EXIT_FAILURE);
		}

		// Set the socket address struct
		setSocketAddr(m_socketAddress, AF_INET, INADDR_ANY, m_port);

		// Associate socket with a specific IP addr and Port number (sockfd,
		// sockaddr *, addrlen)
		if (bind(m_serverSocket, (sockaddr *)&m_socketAddress,
				 m_socketAddress_len) < 0) {
			throw TcpServerException("Cannot bind socket to address");
			return -1;
		}

		return 0;
	}
} // namespace http