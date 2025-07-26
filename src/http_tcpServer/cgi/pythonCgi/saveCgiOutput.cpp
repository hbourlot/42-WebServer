#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::PythonCgi::saveCgiOutput() {

	char buffer[BUFFER_SIZE] = {0};

	close(_inputPipe[1]);
	close(_outputPipe[1]);
	if ((_bytesReceived = read(this->_outputPipe[0], buffer, 20)) > 0) {
		_output.append(buffer, _bytesReceived);
		std::memset(buffer, 0, BUFFER_SIZE);
	}

	// std::cerr << "BYTESRECEIVE =>>>>>>>>>>>>>>>>> " << _bytesReceived << std::endl;
	if (_bytesReceived == 0) {
		_status[FINISHED_IDX] = true;
		close(_outputPipe[0]);
	} else if (_bytesReceived < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			_status[FINISHED_IDX] = false;
			std::cerr << "CLOSING??111111123123131231312111\n";

			close(_outputPipe[0]);
		} else {
			std::cerr << "[CGI] read() error: " << strerror(errno) << std::endl;
			std::cerr << "CLOSING??222222222222222\n";
			_status[ERROR_IDX] = true;
			close(_outputPipe[0]);
		}
	};
}