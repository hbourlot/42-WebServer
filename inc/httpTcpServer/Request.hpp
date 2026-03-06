#pragma once

#include "Logs/Logs.hpp"
#include "Config/Configs.hpp"

#include <fcntl.h>
#include <unistd.h>

enum REQUEST_PHASE { START, HEADER, BODY, FINISHED };

enum ChunkState { CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, CHUNK_DONE };

struct CLengthParser {
	size_t length;
	size_t bytesRead;
	bool hasLength;
	CLengthParser() : length(0), bytesRead(0), hasLength(false) {
	}
};

struct ChunkParser {
	ChunkState state;
	size_t currentChunkSize;
	size_t bytesReadInChunk;

	ChunkParser() : state(CHUNK_SIZE), currentChunkSize(0), bytesReadInChunk(0) {
	}
};

namespace http {

	class Request {
	  private:
		std::string _method;
		std::string _uri;
		std::string _path;
		std::string _fullPath;
		std::string _serverProtocol;
		std::string _pathInfo; // For Cgi
		std::string _pathTranslated;
		std::map<std::string, std::string> _headers;
		std::string _body;
		bool _bodyInDisk;
		int _bodyFd;
		size_t _bodyFdSize;
		std::string _bodyPath;

		std::string _queryString;
		const Location* _matchLocation;
		const Directory* _fileDirectory;

		REQUEST_PHASE _requestPhase;
		ChunkParser _chunkParser;
		CLengthParser _lengthParser;

	  public:
		Request();
		~Request();

		std::string getFileName();
		int appendBody(const char* buf, size_t len, const ServerConfig& configs);
		int createTempFile(const ServerConfig& configs);
		void cleanup();
		bool writeBodyToFd(int outFd);
		std::string& readALlBody();

		void resetChunkParser();

		bool isBodyInDisk();
		size_t bodyFdSize();

		// ! -- GETTERS
		std::string& getFullPath();
		const std::string& getFullPath() const;
		std::string& getServerProtocol();
		const std::string& getServerProtocol() const;
		std::string& getMethod();
		const std::string& getMethod() const;
		std::string& getBody();
		size_t getBodySize() const;
		REQUEST_PHASE getRequestPhase();
		ChunkParser& getChunkParser();
		CLengthParser& getLengthParser();
		std::map<std::string, std::string>& getHeaders();
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getUri() const;
		std::string& getUri();
		const std::string& getQueryString() const;
		const std::string& getPathInfo() const;
		const std::string& getPathTranslated() const;
		std::string& getPathTranslated();
		const Directory* getFileDirectory() const;
		const Location* getMatchLocation() const;

		// ! -- SETTERS
		void setFullPath(std::string src);
		void setQueryString(std::string src);
		void setUri(const std::string src);
		void setPathInfo(const std::string src);
		void setPathTranslated(const std::string src);
		void setMethod(const std::string src);
		void setServerProtocol(const std::string src);
		void setMatchLocation(const Location* location);
		void setFileDirectory(const Directory* location);
		void setRequestPhase(REQUEST_PHASE requestPhase);
	};

} // namespace http
