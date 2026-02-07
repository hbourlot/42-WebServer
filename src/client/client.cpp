#include "Client/Client.hpp"

Client::Client(int fd, http::TcpServer &server)
    : _server(server), _fd(fd), _state(), _requestComplete(false), _cgiInProgress(false), _cgiPid(-1), _cgiOutputFd(-1),
      _bytesToDiscard(0), _discardingBody(false), _request(/* server.getServerInfo() */) {
}

Client::~Client() {
}

http::TcpServer &Client::getServer() {
	return _server;
}

int Client::getFd() const {
	return (_fd);
}

std::string &Client::getReadBuffer() {
	return (_readBuffer);
}
std::string &Client::getWriteBuffer() {
	return (_writeBuffer);
}
void Client::appendToReadBuffer(const std::string &data) {
	_readBuffer += data;
}
void Client::appendToWriteBuffer(const std::string &data) {
	_writeBuffer += data;
}
void Client::clearBuffers() {
	// _readBuffer.clear();
	// _writeBuffer.clear();
    std::string().swap(_readBuffer);
    std::string().swap(_writeBuffer);
}
void Client::clearReadBuffer() {
	_readBuffer.clear();
}
void Client::clearWriteBuffer() {
	_writeBuffer.clear();
}

http::Request &Client::getRequest() {
	return (_request);
}
http::Response &Client::getResponse() {
	return (_response);
}
void Client::resetRequest() {
	_request.cleanup();
	_request = http::Request();
}
void Client::resetResponse() {
	_response = http::Response();
}

bool Client::isRequestComplete() const {
	return (_requestComplete);
}
void Client::setRequestComplete(bool value) {
	_requestComplete = value;
}

bool Client::isCgiInProgress() const {
	return (_cgiInProgress);
}

void Client::setCgiInProgress(bool value) {
	_cgiInProgress = value;
}

pid_t Client::getCgiPid() const {
	return _cgiPid;
}

void Client::setCgiPid(pid_t pid) {
	_cgiPid = pid;
}

int Client::getCgiOutputFd() const {
	return _cgiOutputFd;
}

void Client::setCgiOutputFd(int fd) {
	_cgiOutputFd = fd;
}

void Client::storeCgiInfo(pid_t pid, int fd) {
}

void Client::setState(CLIENT_STATE state) {
	_state = state;
}

CLIENT_STATE Client::getState() const {
	return _state;
}

void Client::setSessionId(std::string &sessionId) {
	_sessionId = sessionId;
}

std::string Client::getSessionId() const {
	return (_sessionId);
}

void Client::consumeReadBuffer(size_t n) {
	if (n >= _readBuffer.size())
		_readBuffer.clear();
	else
		_readBuffer.erase(0, n);
}

size_t Client::getBytesToDiscard() {
	return (_bytesToDiscard);
}
void Client::setBytesToDiscard(size_t bytesToDiscard) {
	_bytesToDiscard = bytesToDiscard;
}
bool Client::getDiscardingBody() {
	return (_discardingBody);
}
void Client::setDiscardingBody(bool discardingBody) {
	_discardingBody = discardingBody;
}
