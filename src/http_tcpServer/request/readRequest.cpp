#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

bool http::TcpServer::readRequest(std::vector<pollfd> &fds, int i)
{
	static std::map<int, std::string> buffers;
	const size_t CLIENT_MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd = fds[i].fd;

	ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE);
	if (bytesReceived <= 0)
	{
		if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error: read()\n";

		setResponseError(HTTP_BAD_REQ); // o 408?
		// setResponse();
		fds[i].events |= POLLOUT;
		buffers.erase(fd);
		return true;
	}

	buffers[fd].append(buffer, bytesReceived);

	ParseStatus status =
	    parseRequest(_request, buffers[fd], _infos, CLIENT_MAX_BODY_SIZE);

	if (status == PARSE_INCOMPLETE)
		return false;

	if (status == PARSE_TOO_LARGE)
	{
		setResponseError(HTTP_PAYLOAD);
		// setResponse();
		// std::cout << _serverMessage << std::endl;
		fds[i].events |= POLLOUT;
		buffers.erase(fd);
		// return true;
		return false;
	}

	fds[i].events |= POLLOUT;
	buffers.erase(fd);
	return false;
}