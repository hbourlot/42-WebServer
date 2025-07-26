#include "Client/Client.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/poll.h>
#include <vector>

namespace http {

	Client::Client(SocketFD fd, sockaddr_in &socketAddress, std::vector<pollfd> &fds, const Server serverInfo)
	    : _fd(fd), _socketAddress(socketAddress), _fds(fds), _serverInfo(serverInfo), _requestComplete(false),
	      _cgiInProgress(false) {
	}

	Client::~Client() {
		if (this->hasCgi())
			delete _cgi;
	}

	SocketFD Client::getFd() const {
		return (_fd);
	}

	ICgi *Client::getCgi() const {
		return (_cgi);
	}

	std::vector<pollfd> &Client::getFdsLoop() {
		return this->_fds;
	}

	void Client::addCgi(ICgi *object) {
		this->_cgi = object;
	}

	void Client::setPOLLOUT() {
		for (int i = 0; i < _fds.size(); ++i) {
			if (_fds[i].fd == _fd) {
				_fds[i].events |= POLLOUT;
			}
		}
	}

	void Client::executeCgi() const {
		if (!this->hasCgi()) {
			std::cerr << "Client has no Cgi" << std::endl;
			return;
		}
		this->_cgi->execute();
	}

	bool Client::hasCgi() const {
		if (this->_cgi)
			return true;
		return false;
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
		return (_request);
	}
	httpResponse &Client::getResponse() {
		return (_response);
	}
	void Client::resetRequest() {
		// To be set
	}
	void Client::resetResponse() {
		// To be set
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

} // namespace http
