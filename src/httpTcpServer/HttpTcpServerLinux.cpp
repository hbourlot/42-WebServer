#include "httpTcpServer/HttpTcpServerLinux.hpp"

namespace http
{

	TcpServer::TcpServer(ServerConfig server)
	    : _serverInfo(server), _serverSocket(), _socketAddress_len(sizeof(sockaddr_in))
	{
		// this->startServer();
		std::string msg("CREATED SERVER ");
		msg = msg + _serverInfo.host + ":";
		msg += to_str(_serverInfo.port);
		Logs::log(INFO, msg);
	}

	TcpServer::~TcpServer()
	{
		close(_serverSocket);
		// close(_acceptSocket);
		// exit(1); //TODO Exit with a failure code??
	}

} // namespace http