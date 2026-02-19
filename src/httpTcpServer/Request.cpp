#include "httpTcpServer/Request.hpp"
#include <fcntl.h>

http::Request::Request() : _bodyInDisk(false), _bodyFd(-1), _requestPhase(START) {
}

http::Request::~Request() {
}

void http::Request::cleanup() {
	if (_bodyInDisk && _bodyFd != -1) {
		close(_bodyFd);
		remove(_bodyPath.c_str());
	}

	_method.clear();
	_uri.clear();
	_serverProtocol.clear();
	_pathInfo.clear();
	_pathTranslated.clear();
	_headers.clear();

	_body.clear();
	_bodyInDisk = false;
	_bodyFd = -1;
	_bodyFdSize = 0;
	_bodyPath.clear();

	_queryString.clear();
	_matchLocation = NULL;
	_fileDirectory = NULL;

	_requestPhase = START;
	_chunk = ChunkParser();
}

int http::Request::appendBody(const char *buf, size_t len, const ServerConfig &configs) {

	size_t maxBuffer = _matchLocation ? _matchLocation->max_buffer_size : configs.max_buffer_size;
	if (!_bodyInDisk && len > maxBuffer - _body.size()) {
		_bodyInDisk = true;
		if (createTempFile(configs))
			return (-1);

		if (writeAll(_bodyFd, _body.c_str(), _body.size()) < 0)
			return (-1);

		_bodyFdSize = _body.size();
		_body.clear();
	}
	if (_bodyInDisk) {
		if (writeAll(_bodyFd, buf, len) < 0)
			return (-1);
		_bodyFdSize += len;
	} else
		_body.append(buf, len);

	return (0);
}

int http::Request::createTempFile(const ServerConfig &configs) {
	static long requestNbr = 0;
	_bodyPath = joinPath(configs.temp_path, "/webserv_body_" + ft_to_string(requestNbr));
	requestNbr = (requestNbr < 2000) ? requestNbr + 1 : 0;

	_bodyFd = open(_bodyPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
	if (_bodyFd < 0) {
		Logs::log(LOGS_ERROR, "Couldn't create body temp file");
		return -1;
	}
	return 0;
}

bool http::Request::writeBodyToFd(int outFd) {
	if (!_bodyInDisk) {
		ssize_t w = write(outFd, _body.c_str(), _body.size());
		return w >= 0 && (size_t)w == _body.size();
	}

	char buf[8192];
	ssize_t r;

	lseek(_bodyFd, 0, SEEK_SET);
	while ((r = read(_bodyFd, buf, sizeof(buf))) > 0) {
		ssize_t w = write(outFd, buf, r);
		if (w < 0 || w != r) {
			return false;
		}
	}

	return r == 0;
}

std::string &http::Request::readALlBody() {
	if (!_bodyInDisk)
		return (_body);

	_body.clear();
	_body.reserve(_bodyFdSize);

	off_t oldPos = lseek(_bodyFd, 0, SEEK_CUR);

	if (lseek(_bodyFd, 0, SEEK_SET) < 0)
		return _body;

	char buffer[8192];
	ssize_t bytes;

	while ((bytes = read(_bodyFd, buffer, sizeof(buffer))) > 0)
		_body.append(buffer, bytes);

	if (oldPos >= 0)
		lseek(_bodyFd, oldPos, SEEK_SET);

	return _body;
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