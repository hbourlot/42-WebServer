#include "httpTcpServer/EventProcessor.hpp"

http::EventProcessor::EventProcessor(std::vector<pollfd> &allFds, std::vector<TcpServer *> servers)
    : _allSockets(allFds), _servers(servers)
{

	_serverSocketSize = _allSockets.size();
};

http::EventProcessor::~EventProcessor() {};

void http::EventProcessor::run()
{

	int timeOut = 1 * 60 * 1000; // 10s

	try
	{
		while (getStopServer() == false)
		{
			int ret = poll(_allSockets.data(), _allSockets.size(), timeOut);

			if (ret < 0)
				std::cerr << "poll() failed" << std::endl;
			else if (ret == 0)
			{
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				break;
			}

			// Checking for new Connections
			acceptConnections();
			for (size_t i = _serverSocketSize; i < _allSockets.size(); ++i)
			{
				bool erased = removeDeadConnections(i);
				if (erased)
					continue;
				this->processClientEvents(i);
				checkIdleConnections(i);
			}
		}
	}
	catch (ClientEventProcessorException &e)
	{
		std::cerr << "Error handling client connection => " << e.what() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	}

	shutDownProcessor();
	return;
}

void http::EventProcessor::acceptConnections()
{

	SocketFD fd;
	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	for (size_t i = 0; i < _serverSocketSize; ++i)
	{
		while (_allSockets[i].revents & POLLIN)
		{
			unsigned int socketAddress_len = sizeof(sockaddr_in);
			fd = accept(_allSockets[i].fd, (struct sockaddr *)&socketAddress, &socketAddress_len);

			if (fd < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					// Means no more connections to accept
					break;
				}
				Logs::logAcceptError(socketAddress);
				return;
			}
			else
			{

				// Set client socket to non-blocking
				fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

				client_pollfd.fd = fd;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;

				_allSockets.push_back(client_pollfd);

				_servers[i]->setSocketAddress(fd, socketAddress);
				_clientManager.addClient(fd, (*_servers[i]));

				std::string msg("Connection Accepted 🟩 ");
				msg += ft_to_string(client_pollfd.fd);
				Logs::log(LOGS_INFO, msg);
			}
		}
	}
}

bool http::EventProcessor::removeDeadConnections(size_t &index)
{

	if (_allSockets[index].revents & (POLLHUP | POLLERR | POLLNVAL))
	{
		SocketFD fd = _allSockets[index].fd;

		if (_cgi_by_fd.find(fd) != _cgi_by_fd.end())
		{
			return false;
		}
		Client *client = _clientManager.getClient(fd);

		if (client && client->getCgiOutputFd() != -1)
		{
			std::map<int, http::Cgi *>::iterator it = _cgi_by_fd.find(client->getCgiOutputFd());
			if (it != _cgi_by_fd.end())
			{
				this->cleanupCgi(it->second);
			}
		}

		if (client->getServer().getSocketAddressRef().count(fd))
			client->getServer().getSocketAddressRef().erase(fd);

		std::string msg("Closing Dead FD => ");
		msg += ft_to_string(fd);

		Logs::log(LOGS_ERROR, msg);

		_clientManager.removeClient(fd);

		_allSockets.erase(_allSockets.begin() + index);
		close(fd);
		return true;
	}
	return false;
};

static void cleanupAllCgis(std::map<SocketFD, http::Cgi *> &cgis)
{

	for (std::map<int, http::Cgi *>::iterator it = cgis.begin(); it != cgis.end(); ++it)
	{
		it->second->killProcess();
		delete it->second; // Cgi destructor closes pipes
	}
	cgis.clear();
	Logs::log(LOGS_INFO, "Cleaned up all CGI processes");
}

void http::EventProcessor::shutDownProcessor()
{
	Logs::log(LOGS_INFO, "===== Starting to shut down the Server =====");

	// Close all CGI pipes before shutting down
	cleanupAllCgis(_cgi_by_fd);

	for (size_t i = 0; i < _allSockets.size(); ++i)
	{
		std::string msg = "Removing from poll vector at idx '" + ft_to_string(i);
		msg += "' fd='";
		msg += ft_to_string(_allSockets[i].fd);
		msg += "'";
		Logs::log(LOGS_INFO, msg);

		if (_allSockets[i].fd != -1)
			close(_allSockets[i].fd);
		_allSockets.erase(_allSockets.begin() + i);
		--i;
	}
	Logs::log(LOGS_INFO, "===== END =====");
}

void http::EventProcessor::processRead(pollfd &pfd, Client *client, Cgi *cgi)
{

	if (cgi)
	{
		readFromCgi(pfd.fd, cgi->getReadBuffer(), cgi->getState());
		return;
	}

	if (!readFromSocket(pfd.fd, client->getReadBuffer(), client->getState()))
	{
		return;
	}

	if (client->getState() == READ_EMPTY)
		client->setLastAction();

	if (!parseRequestData(*client, client->getServer().getServerInfo()))
	{
		return;
	}

	setSession(client);

	pfd.events = POLLOUT; // Setting to POLL OUT
};

void http::EventProcessor::processWrite(pollfd &pfd, Client *client, int index)
{

	if (client->getState() == CGI_JUST_STARTED || client->getState() == CGI_COMPLETED)
	{
		handleCgiIO(client);
	}
	else if (client->getState() != CGI_COMPLETED)
	{
		if (!processRequest(*client))
			return;
	}

	if (handleResponse(pfd, *client))
	{
		this->closeClientConnection(index);
	}
};

void http::EventProcessor::processClientEvents(int index)
{

	int fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient(fd);

	std::map<int, Cgi *>::iterator it = _cgi_by_fd.find(fd);
	Cgi *cgi = (it != _cgi_by_fd.end()) ? it->second : nullptr;

	if (cgi)
	{
		client = cgi->getClient();
	}

	if (_allSockets[index].revents & POLLIN)
	{
		processRead(_allSockets[index], client, cgi);
	}

	if (_allSockets[index].revents & POLLOUT)
	{
		processWrite(_allSockets[index], client, index);
	}

	if ((_allSockets[index].revents & POLLHUP) && cgi && cgi->hasFinished())
	{
		readFromCgi(fd, cgi->getReadBuffer(), cgi->getState());
		client->setState(CGI_COMPLETED);
		return;
	}
}

void http::EventProcessor::registerCgi(http::Cgi *cgi)
{
	int outputFd = cgi->getOutputPipeFd();

	if (outputFd != -1)
	{

		_cgi_by_fd[outputFd] = cgi; // Add CGI to map

		// Handling Output Pipe (Reading from CGI)
		pollfd pfd;

		pfd.fd = cgi->getOutputPipe()[0];
		pfd.events = POLLIN;
		pfd.revents = 0;
		_allSockets.push_back(pfd);

		std::string msg("Registered CGI for PID ");
		msg += ft_to_string(cgi->getPid());
		msg += " and Client fd=" + ft_to_string(cgi->getClient()->getFd());
		msg += " ";
		msg += " with output fd ";
		msg += ft_to_string(outputFd);
		Logs::log(LOGS_INFO, msg);
	}
}

void http::EventProcessor::cleanupCgi(http::Cgi *cgi)
{
	int outputFd = cgi->getOutputPipeFd();
	Client *client = cgi->getClient();

	cgi->killProcess(); // Kill CGI process if still running

	// Remove CGI pipe fd from poll array BEFORE deleting Cgi (which closes pipes)
	for (size_t i = 0; i < _allSockets.size(); ++i)
	{
		if (_allSockets[i].fd == outputFd)
		{
			_allSockets.erase(_allSockets.begin() + i);
			break;
		}
	}

	// Remove from map
	_cgi_by_fd.erase(outputFd);

	// Reset client CGI state
	if (client)
	{
		client->setCgiPid(-1);
		client->setCgiOutputFd(-1);
	}

	// Delete Cgi object (destructor closes pipes)
	delete cgi;

	std::string msg("Cleaned up CGI with output fd ");
	msg += ft_to_string(outputFd);
	Logs::log(LOGS_INFO, msg);
}

// --- GETTERS

SessionManager &http::EventProcessor::getSessionManager()
{
	return _sessionManager;
}

void http::EventProcessor::checkIdleConnections(size_t index)
{

	SocketFD fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient(fd);

	if (!client)
		return;

// 	if (client->getState() == READ_EMPTY && getActualTime() - client->getLastAction() >
// 	                                            static_cast<long>(client->getServer().getServerInfo().alive_timeout))
// 	{
// 		std::cout << "Close \n";
// 		closeClientConnection(index);
// 	}
}

void http::EventProcessor::closeClientConnection(size_t index)
{
	SocketFD fd = _allSockets[index].fd;
	Client *client = _clientManager.getClient(fd);

	if (client && client->getCgiPid() != -1)
	{
		std::map<int, http::Cgi *>::iterator it = _cgi_by_fd.find(fd);
		if (it != _cgi_by_fd.end())
		{
			delete it->second;
			_cgi_by_fd.erase(it);
		}
	}

	std::string msg("Closing FD => ");
	msg += ft_to_string(fd);

	Logs::log(LOGS_WARN, msg);

	if (client)
	{
		client->getServer().getSocketAddressRef().erase(fd);
	}
	_clientManager.removeClient(fd);
	_allSockets.erase(_allSockets.begin() + index);
	close(fd);
}

void http::EventProcessor::setSession(Client *client)
{
	ensureSessionId(*client);

	std::string requestedSessionId = client->getSessionID();

	Session *session = nullptr;
	if (requestedSessionId.empty())
	{
		session = &_sessionManager.createSession();
	}
	else
	{

		session = &_sessionManager.getSession(requestedSessionId);
	}

	client->setSessionID(session->getSessionId());

	if (session->getSessionId() != requestedSessionId)
	{
		client->getResponse().addToHeader("Set-Cookie", "sessionId=" + session->getSessionId() + "; Path=/; HttpOnly");
	}
}

void http::EventProcessor::readFromCgi(SocketFD fd, std::string &readBuffer, IN_OUT_STATE &state)
{

	char buffer[BUFFER_SIZE];
	bool dataWasReadInThisCall = false;

	while (true)
	{
		ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE - 1);
		if (bytesReceived > 0)
		{
			readBuffer.append(buffer, bytesReceived);
			dataWasReadInThisCall = true;
			continue;
		}
		else if (bytesReceived == 0)
		{
			state = CGI_COMPLETED;
			break;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				if (dataWasReadInThisCall)
				{
					state = READ_SUCCESS;
				}
				break;
			}
			else
			{
				Logs::log(LOGS_ERROR, "Error reading from CGI pipe");
				state = READ_ERROR;
				break;
			}
		}
	}
}

bool http::EventProcessor::readFromSocket(SocketFD fd, std::string &readBuffer, IN_OUT_STATE &state)
{

	char buffer[BUFFER_SIZE];
	int readCount = 0;
	bool dataReceived = false;

	// Read up to MAX_READS_PER_EVENT times per poll event
	while (readCount < MAX_READS_PER_EVENT)
	{

		std::memset(buffer, 0, BUFFER_SIZE);
		ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE - 1);

		if (bytesReceived > 0)
		{
			readBuffer.append(buffer, bytesReceived);
			dataReceived = true;
			readCount++;
			continue; // Try to read more data
		}

		// if (bytesReceived == 0) {
		// 	break;
		// }

		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// No more data available, this is normal
			break;
		}

		if (bytesReceived == 0 /*  && readCount == 0 */)
		{
			state = READ_EMPTY;
			return false;
		}
		// bytesReceived < 0

		// Fatal error
		Logs::log(LOGS_ERROR, "Error: recv()");
		state = READ_ERROR;
		return false;
	}

	if (dataReceived)
	{
		state = READ_SUCCESS;
		return true;
	}
	state = READ_EMPTY;
	return false;
}

void http::EventProcessor::handleCgiIO(Client *client)
{

	std::map<SocketFD, Cgi *>::iterator it = _cgi_by_fd.find(client->getCgiOutputFd());
	Cgi *cgi = nullptr;
	if (it != _cgi_by_fd.end())
	{
		cgi = it->second;
	}

	if (!cgi)
	{
		return;
	}

	if (client->getState() == CGI_JUST_STARTED)
	{

		std::string &readBuffer = cgi->getReadBuffer();
		client->getResponse().appendCgiChunk(readBuffer);
	}
	else if (client->getState() == CGI_COMPLETED)
	{

		if (!cgi->hasSuccessfullyFinished())
		{
			client->getResponse().buildErrorResponse(HTTP_SERVER_ERR, client->getServer().getServerInfo());
		}
		else
		{
			client->getResponse().appendCgiChunk(cgi->getReadBuffer());

			const std::map<std::string, std::string> &cgiHeaders = client->getResponse().getHeaders();
			std::map<std::string, std::string>::const_iterator itAuth = cgiHeaders.find("X-Authenticated-User");

			if (itAuth != cgiHeaders.end() && !itAuth->second.empty())
			{
				const std::string username = itAuth->second;

				const std::string previousId = client->getSessionID();
				Session &authSession = _sessionManager.getSession(previousId);

				authSession.setSessionData("username", username);
				authSession.setSessionData("authenticated", "true");

				client->setSessionID(authSession.getSessionId());
				client->getResponse().addToHeader("Set-Cookie",
				                                  "sessionId=" + authSession.getSessionId() + "; Path=/; HttpOnly");
			}

			client->getResponse().appendCgiChunk(cgi->getReadBuffer(), true);
		}
		cleanupCgi(cgi);
	}
}

bool http::EventProcessor::processRequest(Client &client)
{

	// Session &session = _sessionManager.getSession(client.getSessionId());
	// client.setSessionId(session.getSessionId());

	IN_OUT_STATE state = client.getState();

	// Handle error states first (build error responses)
	if (state != PARSE_OK)
	{
		this->buildErrorResponse(client, state);
		return true;
	}

	http::Router router(client, *this);
	router.process();
	return true;
}

bool http::EventProcessor::buildErrorResponse(Client &client, IN_OUT_STATE state)
{
	http::Response &response = client.getResponse();

	switch (state)
	{
	case READ_ERROR:
		response.buildErrorResponse(HTTP_SERVER_ERR, client.getServer().getServerInfo());
		return true;
	case READ_EMPTY:
		response.buildErrorResponse(HTTP_BAD_REQ, client.getServer().getServerInfo());
		return true;
	case PARSE_TOO_LARGE:
		response.buildErrorResponse(HTTP_PAYLOAD, client.getServer().getServerInfo());
		return true;
	default:
		response.buildErrorResponse(HTTP_SERVER_ERR, client.getServer().getServerInfo());
		return true;
	}
}

bool http::EventProcessor::handleResponse(pollfd &pfd, Client &client)
{

	SocketFD clientFd = client.getFd();

	// Build response if write buffer is emptysendResponse
	if (client.getWriteBuffer().empty() && !client.getResponse().isChunked())
		client.appendToWriteBuffer(client.getResponse().buildResponseString());
	if (client.getResponse().isChunked())
	{
		std::string chunk = client.getResponse().consumeOutBuffer();
		if (!chunk.empty())
			client.appendToWriteBuffer(chunk);
	}

	std::string &writeBuffer = client.getWriteBuffer();

	if (writeBuffer.empty())
		return 0;
	if (sendResponse(pfd, client))
	{
		return (1);
	}

	// Check if all data was sent
	if (writeBuffer.empty() && client.getResponse().isChunked())
	{
		if (client.getResponse().getchunkState() == CHUNK_FINISHED)
			client.getResponse().markChunkendDone();
	}

	if (writeBuffer.empty() && !client.getResponse().isChunked())
	{
		std::string msg("Server Response sent to client fd='");
		msg += ft_to_string(clientFd) + "' sessionID: " + client.getSessionID();
		if (DEBUG)
		{
			msg += " ";
			msg += client.getRequest().getUri();
		}
		Logs::log(LOGS_INFO, msg);

		if (client.getResponse().shouldCloseConnection())
		{
			_clientManager.resetClientState(clientFd);

			return 1; // Close connection
		}
		_clientManager.resetClientState(clientFd);
		pfd.events = POLLIN; // Reset to read for next request
		return 0;
	}

	// Still have data to send, keep POLLOUT active
	pfd.events |= POLLOUT;
	return 0; // Continue sending in next poll event
}

bool http::EventProcessor::sendResponse(pollfd &pfd, Client &client)
{
	SocketFD clientFd = client.getFd();

	int sendCount = 0;
	std::string &writeBuffer = client.getWriteBuffer();
	// std::cout << "writeBuffer" << writeBuffer << std::endl;
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
