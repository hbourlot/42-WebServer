#include "http_tcpServer/PythonCgi.hpp"
#include <fcntl.h>

void printEnvStrings(std::vector<std::string> &_envStrings) {
	std::cerr << "ENVSTRINGS:" << std::endl;
	for (size_t i = 0; i < _envStrings.size(); ++i) {
		std::cerr << "  [" << i << "]: " << _envStrings[i] << std::endl;
	}
	std::cerr << "END ENVSTRINGS" << std::endl;
}

void debugCgiExec(const char *filePath, char *const argv[], char *const envp[]) {
	std::cerr << "CGI EXEC DEBUG" << std::endl;
	std::cerr << "File path: " << (filePath ? filePath : "NULL") << std::endl;

	std::cerr << "ARGV:" << std::endl;
	for (int i = 0; argv && argv[i]; ++i) {
		std::cerr << "  argv[" << i << "]: " << argv[i] << std::endl;
	}

	std::cerr << "ENVP:" << std::endl;
	for (int i = 0; envp && envp[i]; ++i) {
		std::cerr << "  envp[" << i << "]: " << envp[i] << std::endl;
	}
	std::cerr << "END DEBUG" << std::endl;
}

void http::PythonCgi::handleChildProcess() {

	doDup();

	std::cerr << "estou no child process" << std::endl;
	// this->_filePath = "." + this->_filePath;

	// build argv
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(_filePath.c_str()));
	argv.push_back(NULL);

	std::cerr << "Estou no child process 1" << std::endl;
	// build envp
	std::vector<char *> envp;
	this->_envp.clear();

	for (size_t i = 0; i < _envStrings.size(); ++i) {
		this->_envp.push_back(const_cast<char *>(_envStrings[i].c_str()));
	}
	this->_envp.push_back(NULL);

	// debugCgiExec(this->getFilePath().c_str(), argv.data(), envp.data());
	std::cerr << "FilePath " << this->getFilePath() << std::endl; 
	execve(this->getFilePath().c_str(), argv.data(), envp.data());
	std::cerr << "EXECUTE WRONG\n ";
	_exit(1);
}

namespace http {

	void PythonCgi::execute() {

		std::cout << "Vamos executar o CGI" << std::endl;
		if (pipe(this->_inputPipe) < 0 || pipe(this->_outputPipe) < 0) {
			return this->setErrorStatusWLog("Error: Pipe creating failed");
		}
		this->_pid = fork();
		if (this->_pid < 0) {
			return this->setErrorStatusWLog("Error: Fork failed");
		} else if (this->_pid == 0) {
			handleChildProcess();
		} else {

			close(_inputPipe[1]);
			close(_outputPipe[1]);

			fcntl(_outputPipe[0], F_SETFL, fcntl(_outputPipe[0], F_GETFL, 0) | O_NONBLOCK);
			registerPollFd(this->_client->getFdsLoop());
			registerWithManager();
		}
	}

} // namespace http