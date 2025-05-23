#include "http_tcpServer/http_tcpServer_linux.hpp"

namespace http {

	TcpServer::TcpServer(Configs configs)
		:	configs(configs),
			m_ip_address(configs.servers[0].host), m_port(configs.servers[0].port), m_socket(), m_acceptSocket(),
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