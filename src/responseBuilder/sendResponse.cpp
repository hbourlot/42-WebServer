#include "httpTcpServer/HttpStructs.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <unistd.h>
#include <vector>

int http::TcpServer::sendResponse(pollfd &socket)
{
	SocketFD clientFd = socket.fd;

	Client *client = _clientManager.getClient(clientFd);
	client->appendToWriteBuffer(ResponseBuilder::buildResponseString(client->getResponse(), client->getRequest()));

	std::string &writeBuffer = client->getWriteBuffer();
	// std::cout << "writeBuffer " << writeBuffer << "\n";

	if (writeBuffer.empty())
		return 0;

	ssize_t bytesSent = send(clientFd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);

	if (bytesSent < 0)
	{
		if (errno == EPIPE)
			Logs::log(ERROR, "Client disconnected before response");
		else
			Logs::log(ERROR, "Error sending response to client");
		return 1; // cerrar conexión
	}

	writeBuffer.erase(0, bytesSent);

	if (writeBuffer.empty())
	{
		std::string msg("Server Response sent to client ");
		msg += to_str(clientFd);
		Logs::log(INFO, msg);
		return 0;
	}

	// Si todavía quedan datos, esperar siguiente POLLOUT
	return 2; // aún quedan datos por enviar
}
