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
		{
			std::cerr << "Error: read()\n";
		}
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
		_response.setResponseError("413", "Payload Too Large");
		setResponse();
		std::cout << _serverMessage << std::endl;
		fds[i].events |= POLLOUT;
		buffers.erase(fd);
		return true;
	}

	fds[i].events |= POLLOUT;
	buffers.erase(fd);
	return false;
}