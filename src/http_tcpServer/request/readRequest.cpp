#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

bool http::TcpServer::readRequest(std::vector<pollfd> &fds, int i)
{
	static std::map<int, std::string> buffers;
	const size_t CLIENT_MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd = fds[i].fd;

	bool shouldPollOut = false;
	bool finished = false;

	ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE);
	if (bytesReceived <= 0)
	{
		if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error: read()\n";

		setResponseError(HTTP_BAD_REQ);
		shouldPollOut = true;
		finished = true;
	}
	else
	{
		buffers[fd].append(buffer, bytesReceived);

		ParseStatus status = parseRequest(_request, buffers[fd], _serverInfo,
		                                  CLIENT_MAX_BODY_SIZE);

		if (status == PARSE_TOO_LARGE)
		{
			setResponseError(HTTP_PAYLOAD);
			shouldPollOut = true;
			finished = true;
		}
		else if (status == PARSE_OK)
		{
			handleRequest(fds[i], fds, _socketAddressMap[fd]);
			shouldPollOut = true;
			finished = false;
		}
		else if (status == PARSE_INCOMPLETE)
			return false;
	}

	if (shouldPollOut)
		fds[i].events |= POLLOUT;

	buffers.erase(fd);
	return finished;
}
