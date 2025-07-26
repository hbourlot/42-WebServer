#include "http_tcpServer/PythonCgi.hpp"

void http::PythonCgi::doDup() {

	if (dup2(_inputPipe[0], STDIN_FILENO) == -1)
		_exit(1);
	if (dup2(_outputPipe[1], STDOUT_FILENO) == -1)
		_exit(1);

	// -- Closes ends
	close(_inputPipe[0]);
	// close(_inputPipe[1]); // Doubt of this one
	close(_outputPipe[0]);
	// close(_outputPipe[1]);
}