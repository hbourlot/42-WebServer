#include "http_tcpServer/PythonCgi.hpp"
#include <iostream>
#include <sys/wait.h>

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
bool http::PythonCgi::isRunning() {
	return _status[RUNNING_IDX];
}

void http::PythonCgi::handleWait(pid_t &pid, bool *cgiStatus) {

	int status;

	if (_processResult == -1)
		return;
	_processResult = waitpid(pid, &status, WNOHANG);

	if (_processResult == 0) {
		cgiStatus[ICgi::FINISHED_IDX] = false;
		return;
	} else if (_processResult == pid) {
		cgiStatus[ICgi::FINISHED_IDX] = true;
	} else {
		cgiStatus[ICgi::ERROR_IDX] = true;
	}
}

bool http::PythonCgi::isFinished() {
	int status;
	pid_t result;

	if (this->_status[FINISHED_IDX] || this->_status[ERROR_IDX])
		return true;
	handleWait(_pid, _status);

	return _status[FINISHED_IDX];
}

bool http::PythonCgi::hasError() {

	if (this->_status[FINISHED_IDX] || this->_status[ERROR_IDX])
		return true;
	handleWait(_pid, _status);
	return _status[ERROR_IDX];
}

void http::PythonCgi::setErrorStatus() {
	this->_status[ERROR_IDX] = true;
}

void http::PythonCgi::setErrorStatusWLog(std::string msg) {
	std::cerr << msg << std::endl;
	this->_status[ERROR_IDX] = true;
}
void http::PythonCgi::setFinishedStatus() {
	this->_status[FINISHED_IDX] = true;
}
void http::PythonCgi::setRunningStatus() {
	this->_status[RUNNING_IDX] = true;
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
