#include "Client/Client.hpp"
#include "Logs/Logs.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <sys/socket.h>

Client::Client(int fd, http::TcpServer& server)
    : _server(server), _fd(fd), _state(), _requestComplete(false), _cgiInProgress(false), _cgiPid(-1), _cgiOutputFd(-1),
      _bytesToDiscard(0), _discardingBody(false), _request() {
	_lastAction = getActualTime();
}

Client::~Client() {
	clearBuffers();
	resetRequest();
	resetResponse();

	if (_cgiOutputFd > 0)
		close(_cgiOutputFd);

	if (_cgiPid > 0)
		kill(_cgiPid, SIGKILL);
}

http::TcpServer& Client::getServer() {
	return _server;
}

int Client::getFd() const {
	return (_fd);
}

std::string& Client::getReadBuffer() {
	return (_readBuffer);
}
std::string& Client::getWriteBuffer() {
	return (_writeBuffer);
}
void Client::appendToReadBuffer(const std::string& data) {
	_readBuffer += data;
}
void Client::appendToWriteBuffer(const std::string& data) {
	_writeBuffer += data;
}
void Client::clearBuffers() {
	// _readBuffer.clear();
	// _writeBuffer.clear();
	// std::string().swap(_readBuffer);
	std::string().swap(_writeBuffer);
}

void Client::clearReadBuffer() {
	_readBuffer.clear();
}
void Client::clearWriteBuffer() {
	_writeBuffer.clear();
}

http::Request& Client::getRequest() {
	return (_request);
}
http::Response& Client::getResponse() {
	return (_response);
}
void Client::resetRequest() {
	_request.cleanup();
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

void Client::setState(IN_OUT_STATE state) {
	_state = state;
}

IN_OUT_STATE Client::getState() const {
	return _state;
}

IN_OUT_STATE& Client::getState() {
	return _state;
}

void Client::setSessionID(const std::string& sessionID) {
	_sessionID = sessionID;
}

std::string Client::getSessionID() const {
	return (_sessionID);
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

time_t Client::getLastAction() {
	return (_lastAction);
}
void Client::setLastAction() {
	_lastAction = getActualTime();
}

bool Client::readFromClientSocket() {

	char buffer[BUFFER_SIZE];

	ssize_t bytesReceived = read(_fd, buffer, BUFFER_SIZE);

	if (bytesReceived > 0) {
		_readBuffer.append(buffer, bytesReceived);
		_state = READ_SUCCESS;
		return true;
	} else if (bytesReceived == 0) {
		// Client closed the browser
		_state = READ_EMPTY;
		return false;
	} else {
		// Something went wrong with socket
		Logs::log(LOGS_ERROR, "Error: read() from socket");
		_state = READ_ERROR;
		return false;
	}
}

ssize_t Client::sendToClientSocket() {
	std::string &writeBuffer = getWriteBuffer();
	if (writeBuffer.empty())
		return 0;
	ssize_t bytesSent = send(_fd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);
	
	if (bytesSent < 0)
	return -1;
	
	if (bytesSent > 0) {
		setLastAction();
		writeBuffer.erase(0, bytesSent);
	}

	return bytesSent;
}

void Client::prepareWriteBuffer() {
	if (_writeBuffer.empty() && !_response.isChunked())
		appendToWriteBuffer(_response.buildResponseString());
	if (_response.isChunked()) {
		std::string chunk = _response.consumeOutBuffer();
		if (!chunk.empty())
			appendToWriteBuffer(chunk);
	}
}

void Client::finalizeChunkedWrite() {
	if (_writeBuffer.empty() && _response.isChunked()) {
		if (_response.getchunkState() == CHUNK_FINISHED)
			_response.markChunkendDone();
	}
}

bool Client::parseRequestBuffer(const ServerConfig& serverInfo) {
	return http::RequestParser::parseRequestBuffer(*this, serverInfo);
}