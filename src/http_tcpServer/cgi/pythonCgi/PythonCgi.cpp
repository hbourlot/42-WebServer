#include "http_tcpServer/PythonCgi.hpp"
#include <iostream>
#include <sys/wait.h>

using namespace http;

PythonCgi::PythonCgi(Client *client, ClientManager *manager, std::string &filePath)
    : _type(PYTHON_CGI), _client(client), _manager(manager), _filePath(filePath), _envp(), _output(""), _envStrings(),
      _inputPipe(), _outputPipe(), _status() {
}

PythonCgi::~PythonCgi() {
	close(_inputPipe[0]);
	close(_inputPipe[1]);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
}

// Check Cgi State
bool PythonCgi::isRunning() {
	return _status[RUNNING_IDX];
}

void PythonCgi::handleWait(pid_t &pid, bool *cgiStatus) {

	int status;

	if (_processResult == -1)
		return;
	_processResult = waitpid(pid, &status, WNOHANG);
	std::cerr << "PROCESS RESULT => " << _processResult << std::endl;
	if (_processResult == 0) {
		cgiStatus[ICgi::FINISHED_IDX] = false;
		return;
	} else if (_processResult == pid) {
		cgiStatus[ICgi::FINISHED_IDX] = true;
	} else {
		cgiStatus[ICgi::ERROR_IDX] = true;
	}
}

bool PythonCgi::isFinished() {
	int status;
	pid_t result;

	if (this->_status[FINISHED_IDX] || this->_status[ERROR_IDX])
		return true;
	handleWait(_pid, _status);

	return _status[FINISHED_IDX];
}

bool PythonCgi::hasError() {

	if (this->_status[FINISHED_IDX] || this->_status[ERROR_IDX])
		return true;
	handleWait(_pid, _status);
	return _status[ERROR_IDX];
}

void PythonCgi::setErrorStatus() {
	this->_status[ERROR_IDX] = true;
}

void PythonCgi::setErrorStatusWLog(std::string msg) {
	std::cerr << msg << std::endl;
	this->_status[ERROR_IDX] = true;
}
void PythonCgi::setFinishedStatus() {
	this->_status[FINISHED_IDX] = true;
}
void PythonCgi::setRunningStatus() {
	this->_status[RUNNING_IDX] = true;
}

std::string PythonCgi::getFilePath() const {
	return this->_filePath;
}

httpRequest PythonCgi::getCgiRequest() const {
	return _request;
}

httpResponse PythonCgi::getCgiResponse() const {
	return _response;
}

int PythonCgi::getPollFd() const {
	return _outputPipe[0];
}

SocketFD PythonCgi::getClientFd() const {
	return _client->getFd();
};

std::string PythonCgi::getOutput() { // Interface
	this->saveCgiOutput();
	return _output;
}

CgiFd PythonCgi::getCgiFd() const {
	return this->_outputPipe[0];
}

CGI_TYPE PythonCgi::getCgiType() const {
	return this->_type;
}

void PythonCgi::registerWithManager() {
	_manager->addCgi(this->getCgiFd(), _client);
}

void PythonCgi::registerPollFd(std::vector<pollfd> &fds) const {
	pollfd pfd;

	pfd.fd = _outputPipe[0];
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);
}
