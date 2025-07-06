#include "http_tcpServer/PythonCgi.hpp"

http::PythonCgi::PythonCgi(Client *client, std::string &filePath)
    : _client(client), _filePath(filePath), _envp(), _output(""), _envStrings(), _inputPipe(), _outputPipe(),
      _status() {
}

http::PythonCgi::~PythonCgi() {
	close(_inputPipe[0]);
	close(_inputPipe[1]);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
}

// Check Cgi State
bool http::PythonCgi::isRunning(void) const {
	return _status[RUNNING_IDX];
}

bool http::PythonCgi::isFinished(void) const {
	return _status[FINISHED_IDX];
}

bool http::PythonCgi::hasError(void) const {
	return _status[ERROR_IDX];
}

std::string http::PythonCgi::getFilePath() const {
	return this->_filePath;
}

httpRequest http::PythonCgi::getCgiRequest() const {
	return _request;
}

httpResponse http::PythonCgi::getCgiResponse() const {
	return _response;
}

int http::PythonCgi::getPollFd() const {
	return _outputPipe[0];
}

SocketFD http::PythonCgi::getClientFd() const {
	return _client->getFd();
};

std::string http::PythonCgi::getOutput() const {
	return _output;
}

// ! Might not be necessary
void http::PythonCgi::registerPollFd(std::vector<pollfd> &fds) const {
	pollfd pfd;

	pfd.fd = _outputPipe[0];
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);
}
