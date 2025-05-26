#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <vector>

void http::TcpServer::acceptConnection(std::vector<pollfd> &fds) {

	struct pollfd client_pollfd;

	// Checks the if theres readable data available (event)
	while (fds[0].revents & POLLIN) {

		m_acceptSocket = accept(m_serverSocket, NULL, NULL);
		if (m_acceptSocket < 0) {

			std::ostringstream ss;

			ss << "Server failed to accept incoming connection from =>\n"
				  "[ADDRESS: "
			   << inet_ntoa(m_socketAddress.sin_addr) << "]\n"
			   << "[PORT: " << ntohs(m_socketAddress.sin_port) << "]\n";
			std::cerr << ss.str();
			return;
		} else {

			// Set client socket to non-blocking
			fcntl(m_acceptSocket, F_SETFL,
				  fcntl(m_acceptSocket, F_GETFL, 0) | O_NONBLOCK);

			client_pollfd.fd = m_acceptSocket;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;

			fds.push_back(client_pollfd);
			std::cout << "----- Connection Accepted 🟩\n";
		}
	}
}
