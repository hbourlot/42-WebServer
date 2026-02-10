#pragma once
#include "Config/Configs.hpp"
#include "Logs/Logs.hpp"
#include "httpTcpServer/HttpStatus.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
// #include "utils.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define RANGE_SIZE 8192

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

	  public:
		Response();
		Response(const http::Request &request);

		Response &operator=(const Response &other);
		~Response();

		// function Member
		std::string buildResponseString(void);
		const std::map<std::string, std::string> &getHeaders() const;

		bool shouldCloseConnection();

		/// @brief Sets default HTTP headers including Date, Content-Length, and Connection type
		void setDefaultHeaders();
		void addToHeader(std::string key, std::string value);

		std::string getContentType(const std::string &filePath);
		std::string readFileContent(const std::string &filePath);

		void buildCgiHeaderChunked(const HttpStatusCode &status);
		void buildCgiBodyChunked(const char *buffer, size_t len);

		void buildCgiResponse(const HttpStatusCode &status, const std::string &body, const ServerConfig &server);
		void buildResponse(const HttpStatusCode &status, const std::string &body);
		void buildErrorResponse(const HttpStatusCode &status, const ServerConfig &server);
		void buildRedirect(const HttpStatusCode &status, const std::string &url);
		void buildFileResponse(const HttpStatusCode &status, const std::string &filePath, const ServerConfig &server);
		void buildRangeResponse(const std::string &filePath, const ServerConfig &server, struct ::stat &st);
	};

} // namespace http
