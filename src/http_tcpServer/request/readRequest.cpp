#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>

void http::TcpServer::readRequest(std::vector<pollfd> &fds, int i)
{
	char buffer[BUFFER_SIZE + 1] = {0};
	std::string requestContent;
	while ((bytesReceived = read(fds[i].fd, buffer, BUFFER_SIZE)) > 0)
		requestContent.append(buffer, bytesReceived);

	if (bytesReceived < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{

			std::cerr << "Error: read()\n";
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			return;
		}
	}
	// throw TcpServerException(
	// 	"Failed to read bytes from client socket connection");
	if (!requestContent.empty())
	{
		parseRequest(request, requestContent);
		// Set event POLLOUT
		fds[i].events |= POLLOUT;
	}
}
