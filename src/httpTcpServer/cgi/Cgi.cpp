#include "httpTcpServer/Cgi.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <vector>

http::Cgi::Cgi(const http::Request &request, const ServerConfig &serverInfo, Client *client)
    : _status(), _request(request), _serverInfo(serverInfo), _clientAddress(), _client(client), _envp(), _envStrings(),
      _state(RESET) {

	_filePath = request._matchLocation->cgi_pass;

	buildEnvStrings();
}

http::Cgi::~Cgi() {
	// Close all pipe fds
	if (_inputPipe[0] >= 0)
		close(_inputPipe[0]);
	if (_inputPipe[1] >= 0)
		close(_inputPipe[1]);
	if (_outputPipe[0] >= 0)
		close(_outputPipe[0]);
	if (_outputPipe[1] >= 0)
		close(_outputPipe[1]);

	remove(_bodyFileName.c_str());
}

std::string http::Cgi::getFilePath() const {
	return this->_filePath;
}

pid_t http::Cgi::getPid() const {
	return _pid;
}

int http::Cgi::getStatus() const {
	return _status;
}
int &http::Cgi::getStatus() {
	return _status;
}

const int *http::Cgi::getOutputPipe() const {
	return _outputPipe;
}

Client *http::Cgi::getClient() const {
	return _client;
}

void http::Cgi::doDupTwoWay() {
	dup2(_inputPipe[0], STDIN_FILENO);
	dup2(_outputPipe[1], STDOUT_FILENO);

	// Close all pipe fds - we now use stdin/stdout
	close(_inputPipe[0]);
	close(_inputPipe[1]);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
}

void http::Cgi::closeForTwoWay() {
	// Two-way: parent writes to CGI stdin and reads from stdout
	close(_inputPipe[0]);  // Child reads from stdin
	close(_outputPipe[1]); // Child writes to stdout
	                       // Keep _inputPipe[1] to write to CGI
	                       // Keep _outputPipe[0] to read from CGI

	_inputPipe[0] = -1;
	_outputPipe[1] = -1;
}

void http::Cgi::killProcess() {
	if (_pid > 0) {
		// Check if process is still running
		int status;
		pid_t result = waitpid(_pid, &status, WNOHANG);

		if (result == 0) { // Process still running - kill it
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0); // Reap zombie
		}
	}
}

int http::Cgi::prepareCgiInputFile() {

	_bodyFileName = _serverInfo.temp_path + "_" + _client->getSessionId() + "_content.txt";
	int fileFd;

	fileFd = open(_bodyFileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fileFd < 0) {
		std::cerr << "Failed to open file for writing: " << strerror(errno) << std::endl;
		return -1;
	}

	if (!_request.writeBodyToFd(fileFd)) {
		std::cerr << "Failed to write to file: " << strerror(errno) << std::endl;
		close(fileFd);
		return -1;
	}

	close(fileFd);
	fileFd = -1;

	// Close the original pipe read end before overwriting
	if (_inputPipe[0] >= 0) {
		close(_inputPipe[0]);
	}

	_inputPipe[0] = open(_bodyFileName.c_str(), O_RDONLY);
	if (_inputPipe[0] < 0) {
		std::cerr << "Failed to open file for reading: " << strerror(errno) << std::endl;
		return -1;
	}
	return 0;
}

void http::Cgi::executeCgi() {

	if (pipe(this->_inputPipe) < 0 || pipe(this->_outputPipe) < 0) {
		std::cerr << "Pipe creating failed\n";
		return;
	}

	if (prepareCgiInputFile() != 0) {
		// Close pipes on prepare failure
		close(_inputPipe[0]);
		close(_inputPipe[1]);
		close(_outputPipe[0]);
		close(_outputPipe[1]);
		_inputPipe[0] = _inputPipe[1] = _outputPipe[0] = _outputPipe[1] = -1;
		return;
	}

	this->_pid = fork();

	if (this->_pid < 0) {
		std::cerr << "Fork failed\n";
		_inputPipe[0] = _inputPipe[1] = _outputPipe[0] = _outputPipe[1] = -1;
		return;
	} else if (this->_pid == 0) {

		this->doDupTwoWay();

		// build argv
		std::vector<char *> argv;
		argv.push_back(const_cast<char *>(_filePath.c_str()));
		argv.push_back(NULL);

		// build envp
		std::vector<char *> envp;
		this->_envp.clear();

		for (size_t i = 0; i < _envStrings.size(); ++i) {
			this->_envp.push_back(const_cast<char *>(_envStrings[i].c_str()));
		}
		this->_envp.push_back(NULL);

		execve(this->getFilePath().c_str(), argv.data(), this->_envp.data());
		Logs::log(LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() + "': " + strerror(errno));
		_exit(1);
	} else {
		this->closeForTwoWay();
		fcntl(_outputPipe[0], F_SETFL, O_NONBLOCK);
		fcntl(_inputPipe[1], F_SETFL, O_NONBLOCK);
	}
};

IN_OUT_STATE &http::Cgi::getState() {
	return _state;
};

std::string &http::Cgi::getReadBuffer() {
	return _outputBuffer;
};