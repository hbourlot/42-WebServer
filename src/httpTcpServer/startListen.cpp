#include "httpTcpServer/HttpTcpServerLinux.hpp"

void http::TcpServer::startListen()
{
	int listen_fd;

	// TODO: Need to implement a properly valid max number of padding \
	// TODO: connections
	listen_fd = listen(_serverSocket, 10);

	if (listen_fd < 0)
	{
		throw TcpServerException("Socket Listen failed");
	}

	std::ostringstream ss; // Output string stream for logging
	ss << "Listening on ADDRESS: " << inet_ntoa(_socketAddressMap[_serverSocket].sin_addr)
	   << " PORT: " << ntohs(_socketAddressMap[_serverSocket].sin_port);
	Logs::log(INFO, ss.str());
}
