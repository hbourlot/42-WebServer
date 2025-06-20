#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {

	TcpServer::TcpServer(Configs configuration)
	    : _infos(configuration.servers[0]),
	      m_ip_address(configuration.servers[0].host),
	      m_port(configuration.servers[0].port), _serverSocket(),
	      _acceptSocket(), _incomingMessage(), _socketAddress(),
	      _socketAddress_len(sizeof(sockaddr_in)),
	      _serverMessage("") { // Initialize m_serverMessage properly
		                       // this->startServer();
	}

	TcpServer::~TcpServer() {
		close(_serverSocket);
		close(_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http