#pragma once

#include "httpTcpServer/HttpStructs.hpp"
#include <string>
namespace http {
	class TcpServer;
};

enum ChunkState { CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, CHUNK_DONE };

struct ChunkParser {
	ChunkState state;
	size_t currentChunkSize;
	size_t bytesReadInChunk;

	ChunkParser() : state( CHUNK_SIZE ), currentChunkSize( 0 ), bytesReadInChunk( 0 ) {
	}
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

	CGI_JUST_STARTED,
	CGI_IN_EXECUTION,
	CGI_COMPLETED,
};

enum REQUEST_PHASE { START, HEADER, BODY, FINISHED };

class Client {

  public:
	friend class ClientEventProcessor;
	Client( int fd, http::TcpServer &server );
	~Client();

	int getFd() const;

	http::TcpServer &getServer();

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
	http::Request &getRequest();
	http::Response &getResponse();
	void resetRequest();
	void resetResponse();

	// State of CGi REquest
	bool isRequestComplete() const;
	void setRequestComplete( bool value );

	bool isCgiInProgress() const;
	void setCgiInProgress( bool value );

	// SessionID Functions
	std::string getSessionId() const;
	void setSessionId( std::string &sessionId );

	// CGI process tracking
	pid_t getCgiPid() const;
	void setCgiPid( pid_t pid );
	int getCgiOutputFd() const;
	void setCgiOutputFd( int fd );
	void storeCgiInfo( pid_t pid, int fd );

	void consumeReadBuffer( size_t n );

	size_t getBytesToDiscard();
	void setBytesToDiscard( size_t bytesToDiscard );

	bool getDiscardingBody();
	void setDiscardingBody( bool discardingBody );

	REQUEST_PHASE getRequestPhase();
	void setRequestPhase( REQUEST_PHASE requestPhase );

	ChunkParser &getChunkParser() {
		return _chunk;
	}
	void resetChunkParser() {
		_chunk = ChunkParser();
	}

  private:
	http::TcpServer &_server;

	int _fd;
	CLIENT_STATE _state;

	std::string _readBuffer;
	std::string _writeBuffer;

	http::Request _request;
	http::Response _response;

	bool _requestComplete;
	bool _cgiInProgress;

	pid_t _cgiPid;
	int _cgiOutputFd;

	std::string _sessionId;

	size_t _bytesToDiscard;
	bool _discardingBody;

	REQUEST_PHASE _requestPhase;

	ChunkParser _chunk;
};

void ensureSessionId( Client &client );