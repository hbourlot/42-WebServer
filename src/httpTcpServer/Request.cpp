#include "httpTcpServer/Request.hpp"
#include <fcntl.h>

#define MAX_BODY_IN_MEMORY 1048576 // 1MB

http::Request::Request() : bodyInDisk(false), bodyFd(-1), _requestPhase(START) {
}

// http::Request::Request(ServerConfig &config) : _configs(config), bodyInDisk(false), bodyFd(-1) {
// }

http::Request::~Request() {
}

void http::Request::cleanup() {
	if (bodyInDisk && bodyFd != -1) {
		close(bodyFd);
		remove(bodyPath.c_str());
	}
}

void http::Request::appendBody(const char *buf, size_t len) {
	if (!bodyInDisk && body.size() + len > MAX_BODY_IN_MEMORY) {
		bodyInDisk = true;
		if (createTempFile()) {
			return;
			//! Send 500 error server;
		}
		std::cout << "Created body fd" << std::endl;
		write(bodyFd, body.c_str(), body.size());
		bodyFdSize = body.size();
		body.clear();
	}
	if (bodyInDisk) {
		write(bodyFd, buf, len);
		bodyFdSize += len;
	}

	else
		body.append(buf, len);
}

int http::Request::createTempFile() {
	static long requestNbr = 0;
	bodyPath = "./tmp/webserv_body_" + ft_to_string(requestNbr);
	requestNbr = (requestNbr < 2000) ? requestNbr + 1 : 0;
	bodyFd = open(bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);

	if (bodyFd < 0) {
		Logs::log(LOGS_ERROR, "Couldn't create body temp file");
		return -1;
	}
	return 0;
}

bool http::Request::writeBodyToFd(int outFd) {
	if (!bodyInDisk) {
		ssize_t w = write(outFd, body.c_str(), body.size());
		return w >= 0 && (size_t)w == body.size();
	}

	char buf[8192];
	ssize_t r;

	lseek(bodyFd, 0, SEEK_SET);
	while ((r = read(bodyFd, buf, sizeof(buf))) > 0) {
		ssize_t w = write(outFd, buf, r);
		if (w < 0 || w != r)
			return false;
	}
	return r == 0;
}

REQUEST_PHASE http::Request::getRequestPhase() {
	return (_requestPhase);
}
void http::Request::setRequestPhase(REQUEST_PHASE requestPhase) {
	_requestPhase = requestPhase;
}
ChunkParser &http::Request::getChunkParser() {
	return _chunk;
}

void http::Request::resetChunkParser() {
	_chunk = ChunkParser();
}