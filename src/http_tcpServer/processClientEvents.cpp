#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::processClientEvents(std::vector<pollfd> &fds)
{

	int fd;
	bool shouldClose;

	for (size_t i = 1; i < fds.size(); ++i)
	{
		fd = fds[i].fd;

		if (fds[i].revents & POLLIN)
			readRequest(fds, i);
		if (fds[i].revents & POLLOUT)
		{
			handleRequest();
			shouldClose = sendResponse(fds[i]);

			// if (shouldClose)
			// 	exit(0); // ! Never should close???

			fds[i].events &= ~POLLOUT;
			if (shouldClose)
			{
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				--i;
				// continue;
			}
			clearResponse(_request, m_serverMessage);
		}
	}
}