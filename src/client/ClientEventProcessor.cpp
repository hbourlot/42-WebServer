#include "Client/ClientEventProcessor.hpp"
// #include "Http/Response.hpp"

static bool isCgirequest(const http::Request &request, const Location &location)
{

	if (location.cgi_extension.empty())
	{ // Checking if location has CGI configured
		return false;
	}

	for (size_t i = 0; i < location.cgi_extension.size(); ++i)
	{
		if (location.cgi_extension[i] == ".cgi")
		{ // ".cgi" accept any kind of cgi
			return true;
		}
	}

	// Extract file extension from the request path
	std::string path = request.path;
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
	{
		return false; // No extension found
	}

	std::string extension = path.substr(dotPos); // Includes the dot (.py, .cgi, etc.)

	// Check if the extension is in the location's CGI extensions
	for (size_t i = 0; i < location.cgi_extension.size(); ++i)
	{
		std::cout << location.cgi_extension[i] << std::endl;
		if (location.cgi_extension[i] == extension)
		{
			return true;
		}
	}

	return false;
}

http::ClientEventProcessor::ClientEventProcessor(TcpServer &server) : _server(server) {};

http::ClientEventProcessor::~ClientEventProcessor() {};

static void discardingBody(Client &client, pollfd &pfd)
{
	size_t available = client.getReadBuffer().size();

	if (available >= client._bytesToDiscard)
	{
		client.consumeReadBuffer(client._bytesToDiscard);
		client._bytesToDiscard = 0;
		client._discardingBody = false;

		client.getResponse() = http::Response(client.getRequest());
		ensureSessionId(client);
		client.setState(PARSE_TOO_LARGE);

		pfd.events &= ~POLLIN;
		pfd.events |= POLLOUT;
	}
	else
	{
		client._bytesToDiscard -= available;
		client.clearReadBuffer();
	}
}

void http::ClientEventProcessor::processRead(pollfd &pfd, Client *client)
{

	if (client->getCgiPid() != -1)
	{
		// std::cout << "CGI_IN_EXECUTION processRead()\n";
		// exit( 0 );
		return;
	}

	if (!readFromSocket(*client))
	{
		return;
	}

	if (client->_discardingBody)
	{
		discardingBody(*client, pfd);
		return;
	}

	if (!parseRequestData(*client, _server._serverInfo))
	{
		return;
	}
	pfd.events = POLLIN;   // Setting to POLL OUT
	pfd.events |= POLLOUT; // Setting to POLL OUT
};

void http::ClientEventProcessor::processWrite(pollfd &pfd, Client *client, int index)
{

	if (client->getCgiPid() == -1 && client->getState() != CGI_COMPLETED)
	{
		if (!processRequest(*client))
			return;
	}

	if (handleResponse(pfd, *client))
	{
		this->closeConnection(index);
	}
};

bool http::ClientEventProcessor::readFromSocket(Client &client)
{

	char buffer[BUFFER_SIZE];
	int fd = client.getFd();
	int readCount = 0;
	bool dataReceived = false;

	// Read up to MAX_READS_PER_EVENT times per poll event
	while (readCount < MAX_READS_PER_EVENT)
	{
		ssize_t bytesReceived = recv(fd, buffer, BUFFER_SIZE, 0);

		if (bytesReceived > 0)
		{
			client.appendToReadBuffer(std::string(buffer, static_cast<size_t>(bytesReceived)));
			dataReceived = true;
			readCount++;
			continue; // Try to read more data
		}

		if (bytesReceived == 0 && readCount == 0)
		{
			// Peer closed connection
			client.setState(READ_EMPTY);
			return false;
		}

		// bytesReceived < 0
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// No more data available, this is normal
			break;
		}

		// Fatal error
		Logs::log(LOGS_ERROR, "Error: recv()");
		client.setState(READ_ERROR);
		return false;
	}

	if (dataReceived)
	{
		client.setState(READ_SUCCESS);
		return true;
	}

	client.setState(READ_EMPTY);
	return false;
}

void http::ClientEventProcessor::closeConnection(size_t index)
{
	sleep(1);
	_server.closeClientConnection(index);
}

bool http::ClientEventProcessor::processRequest(Client &client)
{
	CLIENT_STATE state = client.getState();
	ServerConfig &serverInfo = _server._serverInfo;

	// Handle error states first (build error responses)
	if (state != PARSE_OK)
	{
		this->buildErrorResponse(client, state);
		return true;
	}

	// Handling SuccessfulRequest - from here
	if (this->handleRouteValidation(client))
	{
		return true;
	}

	// Treating execution of request - from here
	http::Request &request = client.getRequest();
	const MatchResult &res = client.getRequest().matchResult;

	// ─────── LOCATION ───────
	const Location &loc = *res.location;
	const File &file = *res.file;
	if (res.location  && res.file)
	{
		if (isCgirequest(request, loc))
			return Router::routeCgiRequest(client, serverInfo, loc, *this);
		else if (!res.file->cgi_pass.empty() && request.method != "GET")
			return Router::routeCgiRequest(client, serverInfo, file, *this);
	}
	if (res.location)
	{
		Router::routeStaticRequest(client, serverInfo, loc);
		return true;
	}

	// ───────── FILE ─────────
	if (res.file)
	{
		Router::routeStaticRequest(client, serverInfo, file);
		return true;
	}
}

bool http::ClientEventProcessor::buildErrorResponse(Client &client, CLIENT_STATE state)
{
	http::Response &response = client.getResponse();

	switch (state)
	{
	case READ_ERROR:
		response.buildErrorResponse(HTTP_SERVER_ERR, _server._serverInfo);
		return true;
	case READ_EMPTY:
		response.buildErrorResponse(HTTP_BAD_REQ, _server._serverInfo);
		return true;
	case PARSE_TOO_LARGE:
		response.buildErrorResponse(HTTP_PAYLOAD, _server._serverInfo);
		return true;
	default:
		response.buildErrorResponse(HTTP_SERVER_ERR, _server._serverInfo);
		return true;
	}
}

bool http::ClientEventProcessor::handleRouteValidation(Client &client)
{
	http::Response &response = client.getResponse();
	ServerConfig &serverInfo = _server._serverInfo;

	VALIDATION_STATUS validationStatus = Router::validateRequest(client, _server._serverInfo);

	switch (validationStatus)
	{

	case VALID_IS_CGI:
		return false;

	case VALID_OK:
		return false; // Continue to routing

	case VALID_NOT_FOUND:
		response.buildErrorResponse(HTTP_NOT_FOUND, serverInfo);
		return true;

	case VALID_REDIRECT_REQUIRED:
		response.buildRedirect(HTTP_MOVED, client.getRequest().matchResult.location->redirection);
		return true;

	case VALID_METHOD_NOT_ALLOWED:
		response.buildErrorResponse(HTTP_FORBID_METHOD, serverInfo);
		return true;

	default:
		response.buildErrorResponse(HTTP_SERVER_ERR, serverInfo);
		return true;
	}
}

void http::ClientEventProcessor::processCgiEvents(int fd, int index)
{
	std::map<int, http::Cgi *>::iterator it = _server._cgiByFd.find(fd);

	if (it != _server._cgiByFd.end())
	{
		if (this->hasCgiFinished(it->second) && _server._fds[index].revents & POLLIN)
			processCgiOutput(it->second, _server._fds[index]);
		return;
	}
	// Logs::log( LOGS_ERROR, "Something bad happened, restart server." );
	return; // Neither client nor CGI pipe - should not happen, skip
}

void http::ClientEventProcessor::processClientEvents(int index)
{

	int status = 0;
	int fd = _server._fds[index].fd;
	Client *client = _server._clientManager.getClient(fd);

	// Check if this fd is a CGI pipe instead of a client socket
	if (!client)
	{
		processCgiEvents(fd, index);
		return;
	}

	// Regular client socket handling
	if (_server._fds[index].revents & POLLIN)
	{
		processRead(_server._fds[index], client);
	}

	if (_server._fds[index].revents & POLLOUT)
	{
		processWrite(_server._fds[index], client, index);
	}
}

bool http::ClientEventProcessor::handleResponse(pollfd &pfd, Client &client)
{
	SocketFD clientFd = client.getFd();

	// Build response if write buffer is emptysendResponse
	if (client.getWriteBuffer().empty())
		client.appendToWriteBuffer(client.getResponse().buildResponseString());

	std::string &writeBuffer = client.getWriteBuffer();

	if (writeBuffer.empty())
		return 0;

	if (sendResponse(pfd, client))
	{
		return (1);
	}

	// Check if all data was sent
	if (writeBuffer.empty())
	{
		std::string msg("Server Response sent to client ");
		msg += ft_to_string(clientFd) + " sessionID: " + client.getSessionId();
		if (DEBUG)
		{
			msg += " ";
			msg += client.getRequest().path;
		}
		Logs::log(LOGS_INFO, msg);

		if (client.getResponse().shouldCloseConnection())
		{
			_server._clientManager.resetClientState(clientFd);
			return 1; // Close connection
		}
		_server._clientManager.resetClientState(clientFd);
		pfd.events = POLLIN; // Reset to read for next request
		return 0;
	}

	// Still have data to send, keep POLLOUT active
	pfd.events |= POLLOUT;
	return 0; // Continue sending in next poll event
}

bool http::ClientEventProcessor::sendResponse(pollfd &pfd, Client &client)
{
	SocketFD clientFd = client.getFd();

	const int MAX_SENDS_PER_EVENT = 3;
	int sendCount = 0;
	std::string &writeBuffer = client.getWriteBuffer();

	// Send up to MAX_SENDS_PER_EVENT times per poll event
	while (sendCount < MAX_SENDS_PER_EVENT && !writeBuffer.empty())
	{
		ssize_t bytesSent = send(clientFd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);

		if (bytesSent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Socket buffer full, will continue later
				pfd.events = POLLOUT;
				return 0; // Keep connection alive, continue sending later
			}

			if (errno == EPIPE)
				Logs::log(LOGS_ERROR, "Client disconnected before response.");
			else
				Logs::log(LOGS_ERROR, "Error sending response to client.");
			return 1; // Close connection
		}

		if (bytesSent == 0)
			break; // Should not happen with send(), but handle gracefully

		writeBuffer.erase(0, bytesSent);
		sendCount++;
	}
	return (0);
}