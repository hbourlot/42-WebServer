#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	TcpServer::TcpServer(Configs configuration, char *envp[])
	    : _infos(configuration.servers[0]),
	      m_ip_address(configuration.servers[0].host),
	      m_port(configuration.servers[0].port), m_serverSocket(),
	      m_acceptSocket(), m_incomingMessage(), m_socketAddress(),
	      m_socketAddress_len(sizeof(m_socketAddress)), m_serverMessage(""),
	      _envp(envp)
	{ // Initialize m_serverMessage properly
	  // this->startServer();
	}

	TcpServer::~TcpServer()
	{
		close(m_serverSocket);
		close(m_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http