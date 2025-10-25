#include "http_tcpServer/PythonCgi.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


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
		std::cerr << "Estou a criar um debug do cgi lets go" << std::endl;

	int fd_debug = open("/tmp/cgi_debug.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd_debug >= 0){
		std::cerr << "Estou a criar um debug do cgi" << std::endl;
		dup2(fd_debug, STDERR_FILENO);
	}

    std::cerr << "estou no child process" << std::endl;

    // usa FullPath como o executável que queres run
    std::string FullPath = "/home/mistery576/42-WebServer/var/www/cgi-bin/hello.py";

    // build argv (argv[0] normalmente é o path do programa)
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(FullPath.c_str()));
    argv.push_back(NULL);

    std::cerr << "Estou no child process 1" << std::endl;

    // build envp local a passar para execve
    std::vector<char*> envpLocal;
    envpLocal.reserve(_envStrings.size() + 1);
    for (size_t i = 0; i < _envStrings.size(); ++i) {
        envpLocal.push_back(const_cast<char*>(_envStrings[i].c_str()));
    }
    envpLocal.push_back(NULL);

    // debug prints - confirma o que vais passar
    std::cerr << "FilePath: " << FullPath << std::endl;
    for (size_t i = 0; i < envpLocal.size() - 1; ++i)
        std::cerr << "env[" << i << "] = " << envpLocal[i] << std::endl;

    std::cerr << "Vou executar agora o cgi" << std::endl;
    execve(FullPath.c_str(), argv.data(), envpLocal.data());

    // se chegar aqui, execve falhou - escreve erro para saber porquê
    perror("execve");
    _exit(1);
}


namespace http {

	// void PythonCgi::execute() {

	// 	std::cout << "Vamos executar o CGI" << std::endl;
	// 	if (pipe(this->_inputPipe) < 0 || pipe(this->_outputPipe) < 0) {
	// 		return this->setErrorStatusWLog("Error: Pipe creating failed");
	// 	}
	// 	this->_pid = fork();
	// 	if (this->_pid < 0) {
	// 		return this->setErrorStatusWLog("Error: Fork failed");
	// 	} else if (this->_pid == 0) {
	// 		handleChildProcess();
	// 	} else {

	// 		close(_inputPipe[1]);
	// 		close(_outputPipe[1]);

	// 		fcntl(_outputPipe[0], F_SETFL, fcntl(_outputPipe[0], F_GETFL, 0) | O_NONBLOCK);
	// 		registerPollFd(this->_client->getFdsLoop());
	// 		registerWithManager();
	// 	}
	// }
	static std::string to_string_98(size_t value) {
    	std::ostringstream oss;
    	oss << value;
    	return oss.str();
	}

	void PythonCgi::execute() {

		std::cout << "Vamos executar CGI" << std::endl;
		const std::string pythonPath = "/usr/bin/python3";
    	const std::string scriptPath = "/home/mistery576/42-WebServer/var/www/cgi-bin/hello.py";

		const std::string requestBody = "name=alice&action=hello";

		int inputPipe[2];  // parent writes -> child reads (stdin)
    	int outputPipe[2]; // child writes (stdout) -> parent reads

		if (pipe(inputPipe) == -1) 
			std::cerr << "Invalide input" << std::endl;

    	if (pipe(outputPipe) == -1)
			std::cerr << "Invalid output" << std::endl;
		
		// Start fork
		pid_t pid = fork();
		if (pid == 0){
			// Redirect stdin to read end of inputPipe
			if (dup2(inputPipe[0], STDIN_FILENO) == -1) 
				std::cerr << "error with input dup2" << std::endl;
			// Redirect stdout to write end of outputPipe
			if (dup2(outputPipe[1], STDOUT_FILENO) == -1)
				std::cerr << "error with output dup2" << std::endl;
			
			// Close original fds after dup2
			close(inputPipe[0]);
			close(outputPipe[1]);

			// Builds argv
			std::vector<char*> argv;
			argv.push_back(const_cast<char*>(pythonPath.c_str()));     // argv[0] == "python3"
			argv.push_back(const_cast<char*>(scriptPath.c_str()));     // argv[1] == "/path/to/script.py"
			argv.push_back(NULL);

			// Build envp: typical CGI vars (add others as needed)
			std::vector<std::string> envStrings;
			envStrings.push_back("REQUEST_METHOD=POST");
			envStrings.push_back(std::string("CONTENT_LENGTH=") + to_string_98(requestBody.size()));
			envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
			envStrings.push_back("QUERY_STRING=");
			envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");

			std::vector<char*> envp;
			envp.reserve(envStrings.size() + 1);
			for (size_t i = 0; i < envStrings.size(); ++i)
				envp.push_back(const_cast<char*>(envStrings[i].c_str()));
			envp.push_back(NULL);

			// Execve: substitui o processo filho; se falhar, imprime erro (irá para parent's pipe)
			execve(pythonPath.c_str(), argv.data(), envp.data());
			// se chegar aqui, execve falhou
			std::cerr << "excve failed" << std::endl;
			_exit(1);
		}

		close(inputPipe[0]);   // parent doesn't read from inputPipe
        close(outputPipe[1]);  // parent doesn't write to outputPipe

        // 1) Escreve o corpo do pedido para o STDIN do filho
        ssize_t written = 0;
        const char* buf = requestBody.c_str();
        size_t toWrite = requestBody.size();
        while (toWrite > 0) {
            ssize_t w = write(inputPipe[1], buf + written, toWrite);
            if (w == -1) {
                if (errno == EINTR) continue;
                perror("write to child stdin");
                break;
            }
            written += w;
            toWrite -= w;
        }
        // Fecha o lado de escrita para sinalizar EOF ao filho
        close(inputPipe[1]);

        // 2) Lê tudo o que o filho escreve (stdout + stderr) até EOF
        char readBuf[4096];
        ssize_t n;
        std::string childOutput;
        while ((n = read(outputPipe[0], readBuf, sizeof(readBuf))) > 0) {
            childOutput.append(readBuf, n);
        }
        if (n == -1) {
            perror("read from child stdout");
        }
        close(outputPipe[0]);

        // 3) Espera pelo processo filho terminar
        int status = 0;
        waitpid(pid, &status, 0);

        std::cout << "Sai do filho" << std::endl;
        // 4) Mostra o output do CGI (o que enviaria para o cliente)
        std::cout << "=== CGI raw output START ===\n";
        std::cout << childOutput << std::endl;
        std::cout << "=== CGI raw output END ===\n";

        if (WIFEXITED(status)) {
            std::cout << "Child exited with status: " << WEXITSTATUS(status) << "\n";
        } else if (WIFSIGNALED(status)) {
            std::cout << "Child killed by signal: " << WTERMSIG(status) << "\n";
        }
	}

} // namespace http