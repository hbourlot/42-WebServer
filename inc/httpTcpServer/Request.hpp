#pragma once
#include "Config/Configs.hpp"
#include "utils.hpp"
#include <unistd.h>

enum REQUEST_PHASE { START, HEADER, BODY, FINISHED };

enum ChunkState { CHUNK_SIZE, CHUNK_DATA, CHUNK_CRLF, CHUNK_DONE };

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
	  public:
		Request();
		~Request();

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
		const Location *_matchLocation;
		const Directory *_fileDirectory;

		REQUEST_PHASE _requestPhase;
		ChunkParser _chunk;

		std::map<std::string, std::string> &getHeaders() {
			return _headers;
		};

		std::string getFileName();
		int appendBody(const char *buf, size_t len, const ServerConfig &configs);
		int createTempFile(const ServerConfig &configs);
		void cleanup();
		bool writeBodyToFd(int outFd);
		std::string &readALlBody();

		REQUEST_PHASE getRequestPhase();
		void setRequestPhase(REQUEST_PHASE requestPhase);

		ChunkParser &getChunkParser();
		void resetChunkParser();
	};

} // namespace http
