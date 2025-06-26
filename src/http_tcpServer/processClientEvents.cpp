#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>

void http::TcpServer::closeClient(std::vector<pollfd> &fds, size_t &i)
{
	int fd = fds[i].fd;
	close(fd);
	fds.erase(fds.begin() + i);
	_socketAddress.erase(_socketAddress.begin() + i);
	--i;
}

void http::TcpServer::processClientEvents(std::vector<pollfd> &fds)
{

	int fd;
	bool shouldCloseSend;
	bool shouldCloseRead;
	sockaddr_in *currentAddress;

	for (size_t i = 1; i < fds.size(); ++i)
	{
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = fds[i].fd;
		currentAddress = &_socketAddress[i];
		if (fds[i].revents & POLLIN)
		{
			shouldCloseRead = readRequest(fds, i);
		}
		// std::cout << "Here " << std::endl;
		if (fds[i].revents & POLLOUT)
		{
			// std::cout << "Here " << __func__ << std::endl;

			handleRequest(*currentAddress);
			shouldCloseSend = sendResponse(fds[i]);
			fds[i].events &= ~POLLOUT;
			clearResponse(_request, _serverMessage);
		}
		if (shouldCloseRead || shouldCloseSend)
		{
			closeClient(fds, i);
			continue;
		}
	}
}