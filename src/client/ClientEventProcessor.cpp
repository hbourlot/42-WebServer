#include "Client/ClientEventProcessor.hpp"
// #include "Http/Response.hpp"

http::ClientEventProcessor::ClientEventProcessor(std::vector<pollfd>& allFds, std::vector<TcpServer*> servers)
    : _allSockets(allFds), _servers(servers) {

	_serverSocketSize = _allSockets.size();
};

http::ClientEventProcessor::~ClientEventProcessor(){};

static void discardingBody(Client& client, pollfd& pfd) {
	size_t available = client.getReadBuffer().size();
	size_t bytesToDiscard = client.getBytesToDiscard();

	if (available >= bytesToDiscard) {
		client.consumeReadBuffer(bytesToDiscard);
		client.setBytesToDiscard(0);
		client.setDiscardingBody(false);

		client.getResponse().initFromRequest(client.getRequest());
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

	int timeOut = 1 * 60 * 1000; // 10s

	try {
		while (getStopServer() == false) {
			int ret = poll(_allSockets.data(), _allSockets.size(), timeOut);

			if (ret < 0)
				std::cerr << "poll() failed" << std::endl;
			else if (ret == 0) {
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				break;
			}

			// Checking for new Connections
			acceptConnections();
			for (size_t i = _serverSocketSize; i < _allSockets.size(); ++i) {
				bool erased = removeDeadConnections(i);
				if (erased)
					continue;
				this->processClientEvents(i);
				checkIdleConnections(i);
			}
		}
	} catch (ClientEventProcessorException& e) {
		std::cerr << "Error handling client connection => " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	}

	shutDownProcessor();
	return;
}

void http::ClientEventProcessor::acceptConnections() {

	SocketFD fd;
	struct pollfd client_pollfd;
	struct sockaddr_in socketAddress;

	for (int i = 0; i < _serverSocketSize; ++i) {
		while (_allSockets[i].revents && POLLIN) {
			unsigned int socketAddress_len = sizeof(sockaddr_in);
			fd = accept(_allSockets[i].fd, (struct sockaddr*)&socketAddress, &socketAddress_len);

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

				_servers[i]->setSocketAddress(fd, socketAddress);
				_clientManager.addClient(fd, (*_servers[i]));

				std::string msg("Connection Accepted 🟩 ");
				msg += ft_to_string(client_pollfd.fd);
				Logs::log(LOGS_INFO, msg);
			}
		}
	}
}

bool http::ClientEventProcessor::removeDeadConnections(size_t& index) {

	if (_allSockets[index].revents & (POLLHUP | POLLERR | POLLNVAL)) {
		// if (_allSockets[index].revents & (POLLERR | POLLNVAL)) {
		SocketFD fd = _allSockets[index].fd;

		// Check if this is a CGI pipe fd - skip it (handled by processCgiOutput)
		if (_cgi_by_fd.find(fd) != _cgi_by_fd.end()) {
			return false; // CGI pipes are managed separately
		}

		// Clean up CGI resources if this is a client with active CGI
		Client* client = _clientManager.getClient(fd);

		if (client && client->getCgiOutputFd() != -1) {
			// Find and cleanup the CGI
			std::map<int, http::Cgi*>::iterator it = _cgi_by_fd.find(client->getCgiOutputFd());
			if (it != _cgi_by_fd.end()) {
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

void http::ClientEventProcessor::checkIdleConnections(size_t index) {

	SocketFD fd = _allSockets[index].fd;
	Client* client = _clientManager.getClient(fd);

	if (!client)
		return;

	int max_idle_ticks = client->getServer().getServerInfo().alive_timeout * 1000;

	if (_allSockets[index].revents == POLLIN && client->getState() == READ_EMPTY) {
		client->incrementIdleTicks();
		if (client->getIdleTicks() >= max_idle_ticks) {
			closeClientConnection(index);
		}
		return;
	} else {
		client->resetIdleTicks();
	}
}

void http::ClientEventProcessor::closeClientConnection(size_t index) {
	SocketFD fd = _allSockets[index].fd;
	Client* client = _clientManager.getClient(fd);

	if (client && client->getCgiPid() != -1) {
		std::map<int, http::Cgi*>::iterator it = _cgi_by_fd.find(fd);
		if (it != _cgi_by_fd.end()) {
			delete it->second;
			_cgi_by_fd.erase(it);
		}
	}

	std::string msg("Closing FD => ");
	msg += ft_to_string(fd);

	Logs::log(LOGS_WARN, msg);

	if (client) {
		client->getServer().getSocketAddressRef().erase(fd);
	}
	_clientManager.removeClient(fd);
	_allSockets.erase(_allSockets.begin() + index);
	close(fd);
}

static void cleanupAllCgis(std::map<SocketFD, http::Cgi*>& cgis) {

	for (std::map<int, http::Cgi*>::iterator it = cgis.begin(); it != cgis.end(); ++it) {
		it->second->killProcess();
		delete it->second; // Cgi destructor closes pipes
	}
	cgis.clear();
	Logs::log(LOGS_INFO, "Cleaned up all CGI processes");
}

void http::ClientEventProcessor::shutDownProcessor() {
	Logs::log(LOGS_INFO, "===== Starting to shut down the Server =====");

	// Close all CGI pipes before shutting down
	cleanupAllCgis(_cgi_by_fd);

	for (size_t i = 0; i < _allSockets.size(); ++i) {
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

void http::ClientEventProcessor::setSession(Client* client) {
	Session* session = nullptr;
	const std::string sessionId = client->getSessionId();

	if (sessionId.empty()) {
		session = &_sessionManager.createSession();
	} else {
		session = &_sessionManager.getSession(sessionId);
	}

	if (session) {
		client->setSessionId(session->getSessionId());
		// Now you can also set data on the session if needed
		// session->setSessionData("username", "test");
	}
}

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

	if (!parseRequestData(*client, client->getServer()._serverInfo)) {
		return;
	}

	pfd.events = POLLOUT; // Setting to POLL OUT
};

void http::ClientEventProcessor::handleCgiIO(Client* client) {

	std::map<SocketFD, Cgi*>::iterator it = _cgi_by_fd.find(client->getCgiOutputFd());
	Cgi* cgi = nullptr;
	if (it != _cgi_by_fd.end()) {
		cgi = it->second;
	}

	if (!cgi) {
		return;
	}
	if (client->getState() == CGI_JUST_STARTED) {
		std::string& readBuffer = cgi->getReadBuffer();
		client->getResponse().appendCgiChunk(readBuffer);
	} else if (client->getState() == CGI_COMPLETED) {
		if (!hasCgiSuccessfullyFinished(cgi)) {
			// std::cout << "START WEWE\n\n";
			client->getResponse().buildErrorResponse(HTTP_SERVER_ERR, client->getServer().getServerInfo());
		} else {

			std::string cgiOutput = cgi->getReadBuffer();
			std::cout << "START\n\n";
			std::cout << cgiOutput;

			std::string authHeader = "X-Authenticated-User: ";
			size_t headerPos = cgiOutput.find(authHeader);

			if (headerPos != std::string::npos) {
				size_t usernameStart = headerPos + authHeader.length();
				size_t usernameEnd = cgiOutput.find("\n", usernameStart);
				std::string username = cgiOutput.substr(usernameStart, usernameEnd - usernameStart);

				username.erase(username.find_last_not_of(" \n\r\t") + 1);

				Session& newSession = _sessionManager.createSession();
				newSession.setSessionData("username", username);
				newSession.setSessionData("authenticated", "true");

				client->getResponse().setCookie("sessionId=" + newSession.getSessionId());
			}

			client->getResponse().appendCgiChunk(cgi->getReadBuffer(), true);
			if (client->getResponse().isChunked()) {
				client->getResponse().finishCgiChunked();
			}
		}
		cleanupCgi(cgi);
	}
}

void http::ClientEventProcessor::processWrite(pollfd& pfd, Client* client, int index) {

	if (client->getState() == CGI_JUST_STARTED || client->getState() == CGI_COMPLETED) {
		handleCgiIO(client);
	} else if (/* client->getCgiPid() == -1 && */ client->getState() != CGI_COMPLETED) {
		if (!processRequest(*client))
			return;
	}

	if (handleResponse(pfd, *client)) {
		this->closeClientConnection(index);
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

bool http::ClientEventProcessor::processRequest(Client& client) {

	Session& session = _sessionManager.getSession(client.getSessionId());
	client.setSessionId(session.getSessionId());

	IN_OUT_STATE state = client.getState();

	ServerConfig& serverInfo = client.getServer()._serverInfo;

	// Handle error states first (build error responses)
	if (state != PARSE_OK) {
		this->buildErrorResponse(client, state);
		return true;
	}

	std::cout << client.getRequest().body << std::endl;
	std::string username = session.getSessionData("username");
	bool isAuthenticated = session.getSessionData("authenticated") == "true";

	// std::cout << "uri:" << client.getRequest().uri << std::endl;
	// std::cout << "f: " << client.getRequest().fullPath << std::endl;
	if ((client.getRequest().uri.find("/Dashboard.html") != std::string::npos) && !isAuthenticated) {
		client.getResponse().buildRedirect(HTTP_MOVED, "http://localhost:8003/pages/Services/Services.html");
		return true;
	}

	// std::cout << "m: " << client.getRequest()._method << std::endl;
	http::Router router(client, *this);
	router.process();
	return true;

	return true;
}

bool http::ClientEventProcessor::buildErrorResponse(Client& client, IN_OUT_STATE state) {
	http::Response& response = client.getResponse();

	switch (state) {
	case READ_ERROR:
		response.buildErrorResponse(HTTP_SERVER_ERR, client.getServer()._serverInfo);
		return true;
	case READ_EMPTY:
		response.buildErrorResponse(HTTP_BAD_REQ, client.getServer()._serverInfo);
		return true;
	case PARSE_TOO_LARGE:
		response.buildErrorResponse(HTTP_PAYLOAD, client.getServer()._serverInfo);
		return true;
	default:
		response.buildErrorResponse(HTTP_SERVER_ERR, client.getServer()._serverInfo);
		return true;
	}
}

void http::ClientEventProcessor::processClientEvents(int index) {

	int fd = _allSockets[index].fd;
	Client* client = _clientManager.getClient(fd);

	std::map<int, Cgi*>::iterator it = _cgi_by_fd.find(fd);
	Cgi* cgi = (it != _cgi_by_fd.end()) ? it->second : nullptr;

	if (cgi)
		client = cgi->getClient();

	// Regular client socket handling
	if (_allSockets[index].revents & POLLIN) {
		processRead(_allSockets[index], client, cgi);
	}

	if (_allSockets[index].revents & POLLOUT) {
		processWrite(_allSockets[index], client, index);
	}

	if (cgi && hasCgiFinished(cgi)) {
		client->setState(CGI_COMPLETED);
	}
}

bool http::ClientEventProcessor::handleResponse(pollfd& pfd, Client& client) {

	SocketFD clientFd = client.getFd();

	// Build response if write buffer is emptysendResponse
	if (client.getWriteBuffer().empty() && !client.getResponse().isChunked())
		client.appendToWriteBuffer(client.getResponse().buildResponseString());
	if (client.getResponse().isChunked()) {
		std::string chunk = client.getResponse().consumeOutBuffer();
		if (!chunk.empty())
			client.appendToWriteBuffer(chunk);
	}

	std::string& writeBuffer = client.getWriteBuffer();

	if (writeBuffer.empty())
		return 0;
	if (sendResponse(pfd, client)) {
		return (1);
	}

	// Check if all data was sent
	if (writeBuffer.empty() && client.getResponse().isChunked()) {
		if (client.getResponse().getchunkState() == CHUNK_FINISHED)
			client.getResponse().markChunkendDone();
	}

	if (writeBuffer.empty() && !client.getResponse().isChunked()) {
		std::string msg("Server Response sent to client fd='");
		msg += ft_to_string(clientFd) + "' sessionID: " + client.getSessionId();
		if (DEBUG) {
			msg += " ";
			msg += client.getRequest().uri;
		}
		Logs::log(LOGS_INFO, msg);

		if (client.getResponse().shouldCloseConnection()) {
			client.getServer()._clientManager.resetClientState(clientFd);

			return 1; // Close connection
		}
		client.getServer()._clientManager.resetClientState(clientFd);
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
	// std::cout << "writeBuffer" << writeBuffer << std::endl;
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