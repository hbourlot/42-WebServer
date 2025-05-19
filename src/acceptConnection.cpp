#include "http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::acceptConnection(SOCKET &new_socket) {
		new_socket = accept(m_socket, (sockaddr *)&m_socketAddress,
							&m_socketAddress_len);
		if (new_socket < 0) {
			std::ostringstream ss;
			ss << "Server failed to accept incoming connection from =>\n"
				  "[ADDRESS: "
			   << inet_ntoa(m_socketAddress.sin_addr) << "]\n"
			   << "[PORT: " << ntohs(m_socketAddress.sin_port) << "]\n";
			throw TcpServerException(ss.str());
		} else {
			std::cout << "----- Connection Accepted 🟩" << std::endl;
		}
	}
} // namespace http