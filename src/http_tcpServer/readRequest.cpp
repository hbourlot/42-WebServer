#include "http_tcpServer_linux.hpp"
#include <sys/poll.h>

static void parseRequest(httpRequest &request,
						 const std::string &requestContent) {
	std::istringstream request_stream(requestContent);
	std::string line;
	size_t idx;
	request_stream >> request.method >> request.path >> request.protocol; //
	while (std::getline(request_stream, line)) {
		idx = line.find(":");
		if (idx != std::string::npos) {
			std::string key = line.substr(0, idx);
			std::string value = line.substr(idx + 1);
			key.erase(key.find_first_not_of(" \t\r\n") + 1);
			value.erase(0, value.find_first_not_of(" \t\r\n"));
			request.headers[key] = value;
		}
	}

	std::string body;
	while (std::getline(request_stream, line))
		body += line + "\n";
	request.body = body;
}

void http::TcpServer::readRequest(int fd, std::vector<pollfd> fds, int i) {
	char buffer[BUFFER_SIZE] = {0};

	bytesReceived = read(fd, buffer, BUFFER_SIZE - 1);
	if (bytesReceived < 0) {
		close(fd);
		fds.erase(fds.begin() + i);
		// throw TcpServerException(
		// 	"Failed to read bytes from client socket connection");
	}
	write(2, buffer, BUFFER_SIZE);
	buffer[bytesReceived] = '\0';
	std::string requestContent(buffer);
	parseRequest(request, requestContent);
}
