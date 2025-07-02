#pragma once

#pragma once

// #include "http_tcpServer_linux/HttpStructs.hpp"
#include "http_tcpServer/HttpStructs.hpp"
#include <string>

class Client
{
  public:
	Client(int fd);
	~Client();

	int getFd() const;

	// Buffers
	std::string &getReadBuffer();
	std::string &getWriteBuffer();
	void appendToReadBuffer(const std::string &data);
	void appendToWriteBuffer(const std::string &data);
	void clearBuffers();
	void clearReadBuffer();
	void clearWriteBuffer();

	// request-response structures
	httpRequest &getRequest();
	httpResponse &getResponse();
	void resetRequest();
	void resetResponse();

	// State of CGi REquest
	bool isRequestComplete() const;
	// void setRequestComplete(bool value);

	bool isCgiInProgress() const;
	void setCgiInProgress(bool value);


  private:
	int _fd;

	std::string _readBuffer;
	std::string _writeBuffer;

	httpRequest _request;
	httpResponse _response;

	// bool _requestComplete;
	bool _cgiInProgress;
};
