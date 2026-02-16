#pragma once
#include "Logs/Logs.hpp"
#include "httpTcpServer/HttpStatus.hpp"
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define RANGE_SIZE 8192

enum CgiChunkState { CHUNK_PARSE_HEADERS, CHUNK_STREAM_BODY, CHUNK_FINISHED };

namespace http {
	struct Request;

	class Response {
	  private:
		std::string _protocol;
		std::pair<std::string, std::string> _connectionType;
		std::pair<std::string, std::string> _range;
		std::string _statusCode;
		std::string _statusMsg;
		std::string _body;
		std::map<std::string, std::string> _headers;
		bool _isChunked;
		CgiChunkState _chunkState;
		std::string _outBuffer;

	  public:
		Response();
		// Response(const http::Request &request);
		void initFromRequest(const http::Request& request);
		// Response &operator=(const Response &other);
		~Response();

		// function Member
		std::string buildResponseString(void);
		const std::map<std::string, std::string>& getHeaders() const;

		std::string getBody() {
			return _body;
		}

		void setCookie(std::string v) {
			_headers["Set-Cookie"] = v;
		}

		bool shouldCloseConnection();

		/// @brief Sets default HTTP headers including Date, Content-Length, and Connection type
		void setDefaultHeaders();
		void addToHeader(std::string key, std::string value);

		std::string getContentType(const std::string& filePath);
		std::string readFileContent(const std::string& filePath);

		void buildCgiChunked(const HttpStatusCode& status, const std::string& buffer, const ServerConfig& server);
		void buildCgiHeaderChunked(const HttpStatusCode& status);
		void buildCgiBodyChunked(const char* buffer, size_t len);

		void initChunked();
		void appendCgiChunk(std::string& buffer, bool isFinished = false);
		void appendChunk(std::string& data);
		void finishCgiChunked();
		bool parseCgiHeaders(std::string& buffer);
		std::string& getCgiOutBuffer();
		CgiChunkState getchunkState() const;
		std::string consumeOutBuffer();
		void markChunkendDone();

		void buildCgiResponse(const HttpStatusCode& status, const std::string& body, const ServerConfig& server);
		void buildResponse(const HttpStatusCode& status, const std::string& body);
		void buildErrorResponse(const HttpStatusCode& status, const ServerConfig& server);
		void buildRedirect(const HttpStatusCode& status, const std::string& url);
		void buildFileResponse(const HttpStatusCode& status, const std::string& filePath, const ServerConfig& server);
		void buildRangeResponse(const std::string& filePath, const ServerConfig& server, struct ::stat& st);

		bool isChunked() const;
	};

} // namespace http
