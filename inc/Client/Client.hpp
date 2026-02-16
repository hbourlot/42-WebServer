#pragma once

#include "httpTcpServer/HttpStructs.hpp"

#include <string>
namespace http {
	class TcpServer;
};

enum IN_OUT_STATE {
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

	CGI_JUST_STARTED,
	CGI_IN_EXECUTION,
	CGI_COMPLETED,
};

class Client {

  public:
	friend class ClientEventProcessor;
	Client(int fd, http::TcpServer& server);
	~Client();

	int getFd() const;

	http::TcpServer& getServer();

	// Buffers
	std::string& getReadBuffer();
	std::string& getWriteBuffer();
	void appendToReadBuffer(const std::string& data);
	void appendToWriteBuffer(const std::string& data);
	void clearBuffers();
	void clearReadBuffer();
	void clearWriteBuffer();
	void setState(IN_OUT_STATE state);
	IN_OUT_STATE getState() const;
	IN_OUT_STATE& getState();

	// request-response structures
	http::Request& getRequest();
	http::Response& getResponse();
	void resetRequest();
	void resetResponse();

	// State of CGi REquest
	bool isRequestComplete() const;
	void setRequestComplete(bool value);

	bool isCgiInProgress() const;
	void setCgiInProgress(bool value);

	// SessionID Functions
	std::string getSessionId() const;
	void setSessionId(std::string& sessionId);

	// CGI process tracking
	pid_t getCgiPid() const;
	void setCgiPid(pid_t pid);
	int getCgiOutputFd() const;
	void setCgiOutputFd(int fd);

	void consumeReadBuffer(size_t n);

	size_t getBytesToDiscard();
	void setBytesToDiscard(size_t bytesToDiscard);

	bool getDiscardingBody();
	void setDiscardingBody(bool discardingBody);

	void incrementIdleTicks();
	int getIdleTicks();
	void resetIdleTicks();

	std::string getSessionId() {
		return _sessionID;
	}

	void setSessionId(const std::string& id) {
		_sessionID = id;
	}

	

  private:
	http::TcpServer& _server;

	int _fd;
	std::string _sessionID;
	IN_OUT_STATE _state;

	bool _requestComplete;
	bool _cgiInProgress;
	pid_t _cgiPid;
	int _cgiOutputFd;
	size_t _bytesToDiscard;
	bool _discardingBody;

	http::Request _request;

	std::string _readBuffer;
	std::string _writeBuffer;
	http::Response _response;

	std::string _sessionId;
	int _idleTicks;
};

void ensureSessionId(Client& client);