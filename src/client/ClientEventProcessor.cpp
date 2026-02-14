#include "Client/ClientEventProcessor.hpp"
// #include "Http/Response.hpp"

http::ClientEventProcessor::ClientEventProcessor(TcpServer& server) : _server(server){};

http::ClientEventProcessor::ClientEventProcessor(std::vector<pollfd>& allFds, std::vector<TcpServer*> servers)
    : _allSockets(allFds), _servers(servers) {

	int size_servers = _allSockets.size();
};

http::ClientEventProcessor::~ClientEventProcessor(){};

static void discardingBody(Client& client, pollfd& pfd) {
	size_t available = client.getReadBuffer().size();
	size_t bytesToDiscard = client.getBytesToDiscard();

	if (available >= bytesToDiscard) {
		client.consumeReadBuffer(bytesToDiscard);
		client.setBytesToDiscard(0);
		client.setDiscardingBody(false);

		client.getResponse() = http::Response(client.getRequest());
		ensureSessionId(client);
		client.setState(PARSE_TOO_LARGE);

		pfd.events &= ~POLLIN;
		pfd.events |= POLLOUT;
	} else {
		client.setBytesToDiscard(bytesToDiscard - available);
		client.clearReadBuffer();
	}
}

void http::ClientEventProcessor::run() {

	try {
		while (true) {
			int ret = poll(_allSockets.data(), _allSockets.size(), -1);

			if (ret < 0)
				std::cerr << "poll() failed" << std::endl;
			else if (ret == 0) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				return;
			}

			// Checking for new Connections
			acceptConnections();
			for (size_t i = 1; i < _allSockets.size(); ++i) {
				bool erased = removeDeadConnections(processor, i);
				if (erased)
					continue;
				processor.processClientEvents(i);
				checkIdleConnections(i);
			}
		}
	}
}

void http::ClientEventProcessor::acceptConnections() {

	SocketFD fd;
	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	while (_allSockets[0].revents && POLLIN) {
		unsigned int socketAddress_len = sizeof(sockaddr_in);
		fd = accept(_allSockets[0].fd, (struct sockaddr*)&socketAddress, &socketAddress_len);
		if (fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Means no more connections to accept
				break;
			}
			Logs::logAcceptError(socketAddress);
			return;
		} else {

			// Set client socket to non-blocking
			fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

			client_pollfd.fd = fd;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;

			_allSockets.push_back(client_pollfd);

			_servers[0]->setSocketAddress(fd, socketAddress);
			_clientManager.addClient(fd, (*_servers[0]));

			std::string msg("Connection Accepted 🟩 ");
			msg += ft_to_string(client_pollfd.fd);
			Logs::log(LOGS_INFO, msg);
		}
	}
}

bool http::ClientEventProcessor::removeDeadConnections(size_t& index) {
	
	if (_allSockets[index].revents & (POLLHUP | POLLERR | POLLNVAL)) {
		// if (_allSockets[index].revents & (POLLERR | POLLNVAL)) {
		SocketFD fd = _allSockets[index].fd;

		// Check if this is a CGI pipe fd - skip it (handled by processCgiOutput)
		if (_cgiByFd.find(fd) != _cgiByFd.end()) {
			return false; // CGI pipes are managed separately
		}

		// Clean up CGI resources if this is a client with active CGI
		Client* client = _clientManager.getClient(fd);
		if (client && client->getCgiOutputFd() != -1) {
			// Find and cleanup the CGI
			std::map<int, http::Cgi*>::iterator it = _cgiByFd.find(client->getCgiOutputFd());
			if (it != _cgiByFd.end()) {
				this->cleanupCgi(it->second);
			}
		}

		if (_socketAddressMap.count(fd))
			_socketAddressMap.erase(fd);

		std::string msg("Closing Dead FD => ");
		msg += ft_to_string(fd);

		Logs::log(LOGS_ERROR, msg);

		_clientManager.removeClient(fd);

		_allSockets.erase(_fds.begin() + index);
		// --index;
		close(fd);
		return true;
	}
	return false;
};

void http::ClientEventProcessor::processRead(pollfd& pfd, Client* client, Cgi* cgi) {

	std::string& readBuffer = cgi ? cgi->getReadBuffer() : client->getReadBuffer();

	if (!readFromSocket(pfd.fd, readBuffer, cgi ? cgi->getState() : client->getState())) {
		return;
	}

	if (cgi) {
		return;
	}

	if (client->getDiscardingBody()) {
		discardingBody(*client, pfd);
		return;
	}

	if (!parseRequestData(*client, _server._serverInfo)) {
		return;
	}

	pfd.events = POLLOUT; // Setting to POLL OUT
};

void http::ClientEventProcessor::processWrite(pollfd& pfd, Client* client, int index) {

	std::map<SocketFD, Cgi*>::iterator it = _server._cgiByFd.find(client->getCgiOutputFd());
	Cgi* cgi = nullptr;
	if (it != _server._cgiByFd.end()) {
		cgi = it->second;
	}

	if (client->getState() == CGI_COMPLETED && cgi) {

		std::string& readBuffer = cgi->getReadBuffer();

		if (client->getWriteBuffer().empty())
			client->getResponse().buildCgiResponse(HTTP_OK, readBuffer, _server._serverInfo);
		cleanupCgi(cgi);

	} else if (client->getCgiPid() == -1 && client->getState() != CGI_COMPLETED) {
		if (!processRequest(*client))
			return;
	}

	if (handleResponse(pfd, *client)) {
		this->closeConnection(index);
	}
};

bool http::ClientEventProcessor::readFromSocket(SocketFD fd, std::string& readBuffer, IN_OUT_STATE& state) {

	char buffer[BUFFER_SIZE];
	int readCount = 0;
	bool dataReceived = false;

	// Read up to MAX_READS_PER_EVENT times per poll event
	while (readCount < MAX_READS_PER_EVENT) {

		std::memset(buffer, 0, BUFFER_SIZE);
		ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE - 1);

		if (bytesReceived > 0) {
			readBuffer.append(buffer, bytesReceived);
			dataReceived = true;
			readCount++;
			continue; // Try to read more data
		}

		if (bytesReceived == 0) {
			break;
		}

		if (bytesReceived == 0 && readCount == 0) {
			// Peer closed connection
			state = READ_EMPTY;
			return false;
		}

		// bytesReceived < 0
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// No more data available, this is normal
			break;
		}

		// Fatal error
		Logs::log(LOGS_ERROR, "Error: recv()");
		state = READ_ERROR;
		return false;
	}

	if (dataReceived) {
		state = READ_SUCCESS;
		return true;
	}

	state = READ_EMPTY;
	return false;
}

void http::ClientEventProcessor::closeConnection(size_t index) {
	_server.closeClientConnection(index);
}

bool http::ClientEventProcessor::processRequest(Client& client) {

	IN_OUT_STATE state = client.getState();

	ServerConfig& serverInfo = _server._serverInfo;
	// Handle error states first (build error responses)
	if (state != PARSE_OK) {
		this->buildErrorResponse(client, state);
		return true;
	}
	// Handling SuccessfulRequest - from here
	VALIDATION_STATUS validationStatus;
	if (this->handleRouteValidation(client, validationStatus))
		return true;

	if (validationStatus == VALID_IS_CGI)
		Router::routeCgiRequest(client, serverInfo, *client.getRequest().matchLocation, *this);
	else
		Router::routeStaticRequest(client, serverInfo, *client.getRequest().matchLocation);

	return true;
}

bool http::ClientEventProcessor::buildErrorResponse(Client& client, IN_OUT_STATE state) {
	http::Response& response = client.getResponse();

	switch (state) {
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

bool http::ClientEventProcessor::handleRouteValidation(Client& client, VALIDATION_STATUS& validationStatus) {
	http::Response& response = client.getResponse();
	ServerConfig& serverInfo = _server._serverInfo;
	validationStatus = Router::validateRequest(client);

	switch (validationStatus) {

	case VALID_IS_CGI:
		return false;

	case VALID_OK:
		return false; // Continue to routing

	case VALID_NOT_FOUND:
		response.buildErrorResponse(HTTP_NOT_FOUND, serverInfo);
		return true;

	case VALID_REDIRECT_REQUIRED:
		response.buildRedirect(HTTP_MOVED, client.getRequest().matchLocation->redirection);
		return true;

	case VALID_METHOD_NOT_ALLOWED:
		response.buildErrorResponse(HTTP_FORBID_METHOD, serverInfo);
		return true;
	case VALID_FORBIDDEN:
		response.buildErrorResponse(HTTP_FORBID, serverInfo);
		return true;

	default:
		response.buildErrorResponse(HTTP_SERVER_ERR, serverInfo);
		return true;
	}
}

void http::ClientEventProcessor::processClientEvents(int index) {

	int fd = _server._fds[index].fd;
	Client* client = _server._clientManager.getClient(fd);

	std::map<int, Cgi*>::iterator it = _server._cgiByFd.find(fd);
	Cgi* cgi = (it != _server._cgiByFd.end()) ? it->second : nullptr;

	if (cgi)
		client = cgi->getClient();

	// Regular client socket handling
	if (_server._fds[index].revents & POLLIN) {
		processRead(_server._fds[index], client, cgi);
	}

	if (_server._fds[index].revents & POLLOUT) {
		processWrite(_server._fds[index], client, index);
	}

	if (cgi && hasCgiFinished(cgi)) {
		client->setState(CGI_COMPLETED);
	}
}

bool http::ClientEventProcessor::handleResponse(pollfd& pfd, Client& client) {
	SocketFD clientFd = client.getFd();

	// Build response if write buffer is emptysendResponse
	if (client.getWriteBuffer().empty())
		client.appendToWriteBuffer(client.getResponse().buildResponseString());

	std::string& writeBuffer = client.getWriteBuffer();

	if (writeBuffer.empty())
		return 0;
	if (sendResponse(pfd, client)) {
		return (1);
	}

	// Check if all data was sent
	if (writeBuffer.empty()) {
		std::string msg("Server Response sent to client fd='");
		msg += ft_to_string(clientFd) + "' sessionID: " + client.getSessionId();
		if (DEBUG) {
			msg += " ";
			msg += client.getRequest().path;
		}
		Logs::log(LOGS_INFO, msg);

		if (client.getResponse().shouldCloseConnection()) {
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

bool http::ClientEventProcessor::sendResponse(pollfd& pfd, Client& client) {
	SocketFD clientFd = client.getFd();

	int sendCount = 0;
	std::string& writeBuffer = client.getWriteBuffer();

	// Send up to MAX_SENDS_PER_EVENT times per poll event
	while (sendCount < MAX_SENDS_PER_EVENT && !writeBuffer.empty()) {
		ssize_t bytesSent = send(clientFd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);

		if (bytesSent < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
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