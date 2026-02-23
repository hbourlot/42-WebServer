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

http::Cgi::Cgi(const http::Request& request, const ServerConfig& serverInfo, Client* client)
    : _pid(-1), _status(0), _stdinFd(-1), _request(request), _serverInfo(serverInfo),
      _clientAddress(client && client->getServer().getSocketAddressRef().count(client->getFd())
                         ? client->getServer().getSocketAddressRef()[client->getFd()]
                         : sockaddr_in()),
      _client(client), _envp(), _envStrings(), _state(RESET), _hasFinished(false), bytesRead(0), triesRead(3250) {

	_filePath = request._matchLocation->cgi_pass;
	_outputPipe[0] = -1;
	_outputPipe[1] = -1;

	buildEnvStrings();
}

http::Cgi::~Cgi() {
	// Close CGI fds
	if (_stdinFd >= 0)
		close(_stdinFd);
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
int& http::Cgi::getStatus() {
	return _status;
}

const int* http::Cgi::getOutputPipe() const {
	return _outputPipe;
}

Client* http::Cgi::getClient() const {
	return _client;
}

int http::Cgi::getOutputPipeFd() const {
	return _outputPipe[0];
}

void http::Cgi::dupCgiFds() {
	dup2(_stdinFd, STDIN_FILENO);
	dup2(_outputPipe[1], STDOUT_FILENO);

	// Close original fds in child after dup
	close(_stdinFd);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
	_stdinFd = -1;
	_outputPipe[0] = -1;
	_outputPipe[1] = -1;
}

void http::Cgi::killProcess() {
	if (_pid > 0) {
		// Check if process is still running
		int status;
		pid_t result = waitpid(_pid, &status, WNOHANG);

		if (result == 0) { // Process still running - kill it
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0);
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

	if (_stdinFd >= 0) {
		close(_stdinFd);
	}

	_stdinFd = open(_bodyFileName.c_str(), O_RDONLY, 0644);
	if (_stdinFd < 0) {
		std::cerr << "Failed to open file for reading: " << strerror(errno) << std::endl;
		return -1;
	}
	return 0;
}

void http::Cgi::executeCgi() {

	if (prepareCgiInputFile() != 0) {
		return;
	}

	if (pipe(this->_outputPipe) < 0) {
		std::cerr << "Pipe creating failed\n";
		if (_stdinFd >= 0) {
			close(_stdinFd);
			_stdinFd = -1;
		}
		return;
	}

	this->_pid = fork();

	if (this->_pid < 0) {
		std::cerr << "Fork failed\n"; // !
		if (_stdinFd >= 0) {
			close(_stdinFd);
			_stdinFd = -1;
		}
		close(_outputPipe[0]);
		close(_outputPipe[1]);
		_outputPipe[0] = _outputPipe[1] = -1;
		return;
	} else if (this->_pid == 0) {

		this->dupCgiFds();

		// build argv
		std::vector<char*> argv;
		argv.push_back(const_cast<char*>(_filePath.c_str()));
		argv.push_back(NULL);

		// build envp
		std::vector<char*> envp;
		this->_envp.clear();

		for (size_t i = 0; i < _envStrings.size(); ++i) {
			this->_envp.push_back(const_cast<char*>(_envStrings[i].c_str()));
		}
		this->_envp.push_back(NULL);
		dumpEnvp();

		execve(this->getFilePath().c_str(), argv.data(), this->_envp.data());
		Logs::log(LOGS_ERROR, "CGI execution failed for script '" + this->getFilePath() + "': " + strerror(errno));
		_exit(1);
	} else {
		if (_stdinFd >= 0) {
			close(_stdinFd);
			_stdinFd = -1;
		}
		close(_outputPipe[1]);
		_outputPipe[1] = -1;
		// fcntl(_outputPipe[0], F_SETFL, O_NONBLOCK);
		fcntl(_outputPipe[0], F_SETFL, fcntl(_outputPipe[0], F_GETFL, 0) | O_NONBLOCK);
	}
};

IN_OUT_STATE& http::Cgi::getState() {
	return _state;
};

std::string& http::Cgi::getReadBuffer() {
	return _outputBuffer;
};

void http::Cgi::dumpEnvStrings() const {
	std::cerr << "[CGI envStrings]" << std::endl;
	for (size_t i = 0; i < _envStrings.size(); ++i) {
		std::cerr << _envStrings[i] << std::endl;
	}
}

void http::Cgi::dumpEnvp() const {
	std::cerr << "[CGI envp]" << std::endl;
	for (size_t i = 0; i < _envp.size(); ++i) {
		if (_envp[i] == NULL)
			break;
		std::cerr << _envp[i] << std::endl;
	}
}