#pragma once
#include "Config/Configs.hpp"
#include "utils.hpp"
#include <unistd.h>

struct MatchResult {
	const Location *location;
	const File *file;
};
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
		// Request(ServerConfig &config); // ! In future the location of temp files will come from configFile
		~Request();

		// ServerConfig &_configs;
		std::string _method;
		std::string path;
		std::string serverProtocol;
		std::string pathInfo; // For Cgi
		std::string pathTranslated;
		std::map< std::string, std::string > headers;

		std::string body;
		bool bodyInDisk; // !New
		int bodyFd;      // !New
		size_t bodyFdSize;
		std::string bodyPath;

		std::string queryString;
		MatchResult matchResult;

		REQUEST_PHASE _requestPhase;
		ChunkParser _chunk;

		std::string getFileName();
		void appendBody(const char *buf, size_t len);
		int createTempFile();
		void cleanup();
		bool writeBodyToFd(int outFd);

		REQUEST_PHASE getRequestPhase();
		void setRequestPhase(REQUEST_PHASE requestPhase);

		ChunkParser &getChunkParser();
		void resetChunkParser();
	};

} // namespace http
