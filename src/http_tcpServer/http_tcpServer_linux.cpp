#include "http_tcpServer_linux.hpp"

namespace http {

	TcpServer::TcpServer(std::string ip_address, int port)
		: m_ip_address(ip_address), m_port(port), m_socket(), m_acceptSocket(),
		  m_incomingMessage(), m_socketAddress(),
		  m_socketAddress_len(sizeof(m_socketAddress)),
		  m_serverMessage("") { // Initialize m_serverMessage properly
		this->startServer();
	}

	TcpServer::~TcpServer() {
		close(m_socket);
		close(m_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http