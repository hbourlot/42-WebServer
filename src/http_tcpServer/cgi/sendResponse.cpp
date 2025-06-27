#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::Cgi::sendResponse() {

	ssize_t bytesSend = send(_clientSocket.fd, _outputContent.c_str(),
							 _outputContent.size(), MSG_NOSIGNAL);

	if (bytesSend < 0) {
	}
}