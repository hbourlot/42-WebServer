#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::PythonCgi::saveCgiOutput() {

	char buffer[BUFFER_SIZE] = {0};

	// close(_inputPipe[1]);
	// close(_outputPipe[1]);
	// while ((_bytesReceived = read(this->_outputPipe[0], buffer, http::BUFFER_SIZE)) > 0) {
	// 	_outputContent.append(buffer, _bytesReceived);
	// 	std::memset(buffer, 0, BUFFER_SIZE);
	// }
	_bytesReceived = read(_outputPipe[0], buffer, BUFFER_SIZE);
	// std::cout << "DASDASDASDAS\n";
	std::cout << buffer;

	_output.append(buffer);

	if (_bytesReceived == 0) {
		_status[FINISHED_IDX] = true;
		close(_outputPipe[0]);
	} else if (_bytesReceived < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			_status[FINISHED_IDX] = false;
		} else {
			std::cerr << "[CGI] read() error: " << strerror(errno) << std::endl;
			_status[ERROR_IDX] = true;
			close(_outputPipe[0]);
		}
	};
}