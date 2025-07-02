#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

bool http::TcpServer::readRequest(int index)
{
	const size_t CLIENT_MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd = _fds[index].fd;

	bool shouldPollOut = false;
	bool finished = false;

	Client *client = _clientManager.getClient(fd);
	if (!client)
	{
		std::cerr << "Error: Client not found for fd " << fd << std::endl;
		setResponseError(HTTP_BAD_REQ);
		_fds[index].events |= POLLOUT;
		return true; // Close connection
	}

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
		client->appendToReadBuffer(std::string(buffer, bytesReceived));

		ParseStatus status =
		    parseRequest(client->getRequest(), client->getReadBuffer(), _serverInfo, CLIENT_MAX_BODY_SIZE);

		// if (status == PARSE_TOO_LARGE)
		// {
		// 	setResponseError(HTTP_PAYLOAD);
		// 	shouldPollOut = true;
		// 	finished = true;
		// }
		//! Need more lecture  says that its the max for each request

		if (status == PARSE_OK)
		{
			HttpHandler::handle(*client, _serverInfo);
			client->clearReadBuffer();
			shouldPollOut = true;
			finished = false;
		}
		else if (status == PARSE_INCOMPLETE)
			return false;
	}

	if (shouldPollOut)
		_fds[index].events |= POLLOUT;

	// buffers.erase(fd);
	return finished;
}
