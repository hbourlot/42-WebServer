#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <map>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>

static bool handleCgiSocket(std::map<SocketFD, http::Cgi *> &cgiFdMap,
                            std::vector<pollfd> &fds, pollfd &currentSocket) {

	http::Cgi *cgi;
	if (cgiFdMap[currentSocket.fd]) {
		cgi = cgiFdMap[currentSocket.fd];
		if (cgi->getStatus() != http::Cgi::FINISHED ||
		    cgi->getStatus() != http::Cgi::ERROR) {
			cgi->readCgiOutput();
		} else {
		}
	}

	return true;
}

bool http::TcpServer::readSocket(std::vector<pollfd> &fds, int i) {

	static std::map<int, std::string> buffers;
	const size_t CLIENT_MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
	char buffer[BUFFER_SIZE + 1] = {0};
	SocketFD fd = fds[i].fd;

	ssize_t bytesReceived = read(fd, buffer, BUFFER_SIZE);
	if (bytesReceived <= 0) {
		if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error: read()\n";

		setResponseError(HTTP_BAD_REQ); // o 408?
		// setResponse();
		fds[i].events |= POLLOUT;
		buffers.erase(fd);
		return true;
	}

	buffers[fd].append(buffer, bytesReceived);

	ParseStatus status =
	    parseRequest(_request, buffers[fd], _serverInfo, CLIENT_MAX_BODY_SIZE);

	if (status == PARSE_INCOMPLETE)
		return false;

	if (status == PARSE_TOO_LARGE) {
		setResponseError(HTTP_PAYLOAD);
		fds[i].events |= POLLOUT;
		buffers.erase(fd);
		return true;
	}

	if (status == PARSE_OK)
		handleRequest(fds[i], fds, _socketAddressMap[fd]);

	fds[i].events |= POLLOUT;
	buffers.erase(fd);
	return false;
}