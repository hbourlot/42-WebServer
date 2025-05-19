#include "http_tcpServer_linux.hpp"

namespace http {

	int TcpServer::startServer() {

		// Creates a server socket (IPv4, TCP, 0) (domain, type, protocol);
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socket < 0) {
			throw TcpServerException("Cannot create socket");
			return 1;
		}
		// For inactivate the time wait from OS that block bind again
		int opt = 1;
		if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
			0) {
			log("setsockopt failed");
			close(m_socket);
			exit(EXIT_FAILURE);
		}

		// Set the socket address struct
		m_socketAddress.sin_family = AF_INET;
		m_socketAddress.sin_addr.s_addr =
			INADDR_ANY; // can replace this with a specific IP address if needed
		m_socketAddress.sin_port =
			htons(m_port); // Converts 16-bit integer in host byte order

		// Associate socket with a specific IP addr and Port number (sockfd,
		// sockaddr *, addrlen)
		if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) <
			0) {
			throw TcpServerException("Cannot bind socket to address");
			return 1;
		}

		return 0;
	}
} // namespace http