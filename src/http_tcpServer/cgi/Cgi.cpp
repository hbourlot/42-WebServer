#include "http_tcpServer/Cgi.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <vector>

std::string http::Cgi::getFilePath() const {
	return this->_filePath;
}

httpRequest http::Cgi::getCgiRequest() const {
	return _request;
}

httpResponse http::Cgi::getCgiResponse() const {
	return _response;
}

int http::Cgi::getPollFd() const {
	return _outputPipe[0];
}

http::Cgi::CgiStatus http::Cgi::getStatus() const {
	return _status;
}

std::string http::Cgi::getBody() const {
	return _body;
}

void http::Cgi::registerPollFd(std::vector<pollfd> &fds) const {
	pollfd pfd;

	pfd.fd = _outputPipe[0];
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);
}

void http::Cgi::markAsRunning() {
	this->_status = RUNNING;
}

http::Cgi::Cgi(const httpRequest &request, std::string &filePath,
               const sockaddr_in &clientAddress, const ServerConfig &serverInfo)
    : _request(request), _filePath(filePath), _clientAddress(clientAddress),
      _serverInfo(serverInfo), _envp(), _argv(), _envStrings(), _body(),
      _inputPipe(), _outputPipe(), _clientFD() {

	// Cgi::createValidCgiExtensions();

	// execve
	buildEnvStrings();
	_status = NOT_STARTED;
}

http::Cgi::~Cgi() {
	close(_inputPipe[0]);
	close(_inputPipe[1]);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
}
