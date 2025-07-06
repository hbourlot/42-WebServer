#include "http_tcpServer/PythonCgi.hpp"

void http::PythonCgi::sendResponse() {

	ssize_t bytesSend = send(_client->getFd(), _output.c_str(), _output.size(), MSG_NOSIGNAL);
	if (bytesSend < 0) {
		// Treat error
	}
}