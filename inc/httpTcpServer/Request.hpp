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
		std::string path;
		std::string serverProtocol;
		std::string pathInfo; // For Cgi
		std::string pathTranslated;
		std::map<std::string, std::string> headers;

		std::string body;
		bool bodyInDisk;
		int bodyFd;
		size_t bodyFdSize;
		std::string bodyPath;

		std::string queryString;
		const Location *matchLocation;
		const Directory *fileDirectory;

		REQUEST_PHASE _requestPhase;
		ChunkParser _chunk;

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
