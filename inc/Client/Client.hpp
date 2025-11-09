#pragma once

#include "httpTcpServer/HttpStructs.hpp"
#include <string>
namespace http {
	class TcpServer;
};

enum CLIENT_STATE {
	RESET,
	READ_SUCCESS = 1,
	READ_INCOMPLETE,
	READ_ERROR,
	READ_EMPTY,
	/////
	PARSE_INCOMPLETE,
	PARSE_TOO_LARGE,
	PARSE_OK,
	PARSE_ERROR,

	CGI_IN_EXECUTION,
	CGI_COMPLETED,
};

class Client {

  public:
	friend class ClientEventProcessor;
	Client( int fd, http::TcpServer &server );
	~Client();

	int getFd() const;

	// Buffers
	std::string &getReadBuffer();
	std::string &getWriteBuffer();
	void appendToReadBuffer( const std::string &data );
	void appendToWriteBuffer( const std::string &data );
	void clearBuffers();
	void clearReadBuffer();
	void clearWriteBuffer();
	void setState( CLIENT_STATE state );
	CLIENT_STATE getState() const;

	// request-response structures
	httpRequest &getRequest();
	httpResponse &getResponse();
	void resetRequest();
	void resetResponse();

	// State of CGi REquest
	bool isRequestComplete() const;
	void setRequestComplete( bool value );

	bool isCgiInProgress() const;
	void setCgiInProgress( bool value );

	http::TcpServer& getServer(){
		return _server;
	}

  private:
	int _fd;
	CLIENT_STATE _state;

	std::string _readBuffer;
	std::string _writeBuffer;

	httpRequest _request;
	httpResponse _response;

	bool _requestComplete;
	bool _cgiInProgress;

	http::TcpServer &_server;
};