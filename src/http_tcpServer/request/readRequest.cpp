#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

void http::TcpServer::readRequest(std::vector<pollfd> &fds, int i)
{

	char buffer[BUFFER_SIZE + 1] = {0};
	std::string requestContent;

	while ((_bytesReceived = read(fds[i].fd, buffer, BUFFER_SIZE)) > 0)
		requestContent.append(buffer, _bytesReceived);

	if (_bytesReceived < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{

			std::cerr << "Error: read()\n";
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			return;
		}
	}
	if (!requestContent.empty())
	{
		parseRequest(_request, requestContent, _infos);
		// Set event POLLOUT
		fds[i].events |= POLLOUT;
	}
}
