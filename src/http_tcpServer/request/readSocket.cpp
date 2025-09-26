#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

bool http::TcpServer::readSocket(int index) {

	int fd = _fds[index].fd;
	Client *client = _clientManager.getClient(fd);
	const size_t CLIENT_MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	char buffer[BUFFER_SIZE + 1] = {0};
	bool shouldPollOut = false;
	bool finished = false;

	if (_clientManager.hasCgiClient(fd)) {
		_clientManager.getCgiClient(fd)->getCgi()->getOutput();
		_clientManager.getCgiClient(fd)->setPOLLOUT();
		// _fds[index].events |= POLLOUT;
		return false;
	}

	if (!client) {
		std::cerr << "Error: Client not found for fd " << fd << std::endl;
		client->getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
			std::cout << "[DEBUG] Sending 8" << std::endl;

		client->appendToWriteBuffer(ResponseBuilder::buildResponseString(client->getResponse(), client->getRequest()));
		_fds[index].events |= POLLOUT;
		return true; // Close connection
	}

	ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE);
	if (bytesReceived <= 0) {
		if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error: read()\n";
		client->getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
			// std::cout << "[DEBUG] Sending 9" << std::endl;

		client->appendToWriteBuffer(ResponseBuilder::buildResponseString(client->getResponse(), client->getRequest()));
		shouldPollOut = true;
		finished = true;
	} else {
		client->appendToReadBuffer(std::string(buffer, bytesReceived));

		ParseStatus status = parseRequest(client->getRequest(), client->getReadBuffer(), _serverInfo, CLIENT_MAX_BODY_SIZE);
		// if (status == PARSE_TOO_LARGE)
		// {
		// 	setResponseError(HTTP_PAYLOAD);
		// 	shouldPollOut = true;
		// 	finished = true;
		// }
		//! Need more lecture  says that its the max for each request

		if (status == PARSE_OK) {
			std::cout << "[DEBUG] Query: " << client->getRequest().path << "\n";
			// std::cout << "Parsing esta ok" << std::endl;
			HttpHandler::handle(&_clientManager, *client, _serverInfo);
			client->clearReadBuffer();
			shouldPollOut = true;
			finished = false;
			// std::cout << "Parsing ja saiu" << std::endl;
		} else if (status == PARSE_INCOMPLETE)
		{
			std::cout << "Parse Incomplete" << std::endl;
			return false;
		}
	}

	// std::cout << "ShouldPollout value: " << shouldPollOut << std::endl;
	if (shouldPollOut) {
		// std::cout << "Vamos fechar com o POLLOUT" << std::endl;
		_fds[index].events |= POLLOUT;
	}

	// std::cout << "Ja fechou com o POLLOUT " << std::endl;
	
	// buffers.erase(fd);
	finished = false;
	return finished;
}
