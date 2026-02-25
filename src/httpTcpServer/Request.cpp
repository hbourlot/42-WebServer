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

void http::Request::resetChunkParser() {
	_chunk = ChunkParser();
}

bool http::Request::isBodyInDisk() {
	return _bodyInDisk;
}

size_t http::Request::bodyFdSize() {
	return _bodyFdSize;
}

// ! -- GETTERS

std::string &http::Request::getFullPath() {
	return _fullPath;
}

const std::string &http::Request::getFullPath() const {
	return _fullPath;
}

std::string &http::Request::getServerProtocol() {
	return _serverProtocol;
}

const std::string &http::Request::getServerProtocol() const {
	return _serverProtocol;
}

std::string &http::Request::getMethod() {
	return _method;
}

const std::string &http::Request::getMethod() const {
	return _method;
}

std::string &http::Request::getBody() {
	return _body;
}
size_t http::Request::getBodySize() const {
	std::cout << _bodyFdSize << ":" << _body.size() << std::endl;
	if (_bodyInDisk)
		return (_bodyFdSize);
	return (_body.size());
}

REQUEST_PHASE http::Request::getRequestPhase() {
	return (_requestPhase);
}

ChunkParser &http::Request::getChunkParser() {
	return _chunk;
}

std::map<std::string, std::string> &http::Request::getHeaders() {
	return _headers;
};

const std::map<std::string, std::string> &http::Request::getHeaders() const {
	return _headers;
};

const std::string &http::Request::getUri() const {
	return _uri;
}

std::string &http::Request::getUri() {
	return _uri;
}

const std::string &http::Request::getQueryString() const {
	return _queryString;
}

const std::string &http::Request::getPathInfo() const {
	return _pathInfo;
}

const std::string &http::Request::getPathTranslated() const {
	return _pathTranslated;
}

std::string &http::Request::getPathTranslated() {
	return _pathTranslated;
}

const Directory *http::Request::getFileDirectory() const {
	return _fileDirectory;
}

const Location *http::Request::getMatchLocation() const {
	return _matchLocation;
}

// ! -- SETTERS

void http::Request::setFullPath(std::string src) {
	_fullPath = src;
}

void http::Request::setQueryString(std::string src) {
	_queryString = src;
}

void http::Request::setUri(const std::string src) {
	_uri = src;
}

void http::Request::setPathInfo(const std::string src) {
	_pathInfo = src;
}

void http::Request::setPathTranslated(const std::string src) {
	_pathTranslated = src;
}

void http::Request::setMethod(const std::string src) {
	_method = src;
}

void http::Request::setServerProtocol(const std::string src) {
	_serverProtocol = src;
}

void http::Request::setMatchLocation(const Location *location) {
	_matchLocation = location;
}

void http::Request::setFileDirectory(const Directory *location) {
	_fileDirectory = location;
}

void http::Request::setRequestPhase(REQUEST_PHASE requestPhase) {
	_requestPhase = requestPhase;
}