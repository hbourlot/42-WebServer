#include "Client/Client.hpp"

Client::Client( int fd, http::TcpServer &server )
    : _fd( fd ), _state(), _server( server ), _requestComplete( false ), _cgiInProgress( false ) {
}
Client::~Client() {
}

int Client::getFd() const {
	return ( _fd );
}

std::string &Client::getReadBuffer() {
	return ( _readBuffer );
}
std::string &Client::getWriteBuffer() {
	return ( _writeBuffer );
}
void Client::appendToReadBuffer( const std::string &data ) {
	_readBuffer += data;
}
void Client::appendToWriteBuffer( const std::string &data ) {
	_writeBuffer += data;
}
void Client::clearBuffers() {
	_readBuffer.clear();
	_writeBuffer.clear();
}
void Client::clearReadBuffer() {
	_readBuffer.clear();
}
void Client::clearWriteBuffer() {
	_writeBuffer.clear();
}

httpRequest &Client::getRequest() {
	return ( _request );
}
httpResponse &Client::getResponse() {
	return ( _response );
}
void Client::resetRequest() {
}
void Client::resetResponse() {
}

bool Client::isRequestComplete() const {
	return ( _requestComplete );
}
void Client::setRequestComplete( bool value ) {
	_requestComplete = value;
}

bool Client::isCgiInProgress() const {
	return ( _cgiInProgress );
}

void Client::setCgiInProgress( bool value ) {
	_cgiInProgress = value;
}

void Client::setState(CLIENT_STATE state) {
	_state = state;
}

CLIENT_STATE Client::getState() const {
	return _state;
}