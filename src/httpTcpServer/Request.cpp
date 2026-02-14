#include "httpTcpServer/Request.hpp"
#include <fcntl.h>

http::Request::Request() : bodyInDisk(false), bodyFd(-1), _requestPhase(START) {
}

http::Request::~Request() {
}

void http::Request::cleanup() {
	if (bodyInDisk && bodyFd != -1) {
		close(bodyFd);
		remove(bodyPath.c_str());
	}

	_method.clear();
	path.clear();
	serverProtocol.clear();
	pathInfo.clear();
	pathTranslated.clear();
	headers.clear();

	body.clear();
	bodyInDisk = false;
	bodyFd = -1;
	bodyFdSize = 0;
	bodyPath.clear();

	queryString.clear();
	matchLocation = NULL;
	fileDirectory = NULL;

	_requestPhase = START;
	_chunk = ChunkParser();
}

int http::Request::appendBody(const char *buf, size_t len, const ServerConfig &configs) {

	size_t maxBuffer = matchLocation ? matchLocation->max_buffer_size : configs.max_buffer_size;
	if (!bodyInDisk && body.size() > maxBuffer - len) {
		bodyInDisk = true;
		if (createTempFile(configs))
			return (-1);

		if (writeAll(bodyFd, body.c_str(), body.size()) < 0)
			return (-1);

		bodyFdSize = body.size();
		body.clear();
	}
	if (bodyInDisk) {
		if (writeAll(bodyFd, buf, len) < 0)
			return (-1);
		bodyFdSize += len;
	} else
		body.append(buf, len);

	return (0);
}

int http::Request::createTempFile(const ServerConfig &configs) {
	static long requestNbr = 0;
	bodyPath = joinPath(configs.temp_path, "/webserv_body_" + ft_to_string(requestNbr));
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

std::string &http::Request::readALlBody() {
	if (!bodyInDisk)
		return (body);

	body.clear();
	body.reserve(bodyFdSize);

	off_t oldPos = lseek(bodyFd, 0, SEEK_CUR);

	if (lseek(bodyFd, 0, SEEK_SET) < 0)
		return body;

	char buffer[8192];
	ssize_t bytes;

	while ((bytes = read(bodyFd, buffer, sizeof(buffer))) > 0)
		body.append(buffer, bytes);

	if (oldPos >= 0)
		lseek(bodyFd, oldPos, SEEK_SET);

	return body;
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