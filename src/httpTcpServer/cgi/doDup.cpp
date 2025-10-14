#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <unistd.h>

void http::Cgi::doDup() {
	dup2(_inputPipe[0], STDIN_FILENO);
	dup2(_outputPipe[1], STDOUT_FILENO);

	// -- Closes ends
	close(_inputPipe[1]);
	close(_inputPipe[0]);
	close(_outputPipe[0]);
	// close(_outputPipe[1]);
}