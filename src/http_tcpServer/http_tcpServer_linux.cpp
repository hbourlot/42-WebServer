#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	TcpServer::TcpServer(Server server)
	    : _serverInfo(server), _ipAddress(server.host), _port(server.port),
	      _serverSocket(), _socketAddress_len(sizeof(sockaddr_in)),
	      _serverMessage("")
	{ // Initialize m_serverMessage properly
	  // this->startServer();
	}

	TcpServer::~TcpServer()
	{
		close(_serverSocket);
		// close(_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http