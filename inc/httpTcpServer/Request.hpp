#pragma once
#include "Config/Configs.hpp"
#include "httpTcpServer/HttpStatus.hpp"

struct MatchResult {
	const Location *location;
	const File *file;
};

namespace http {

	struct Request {
		std::string method;
		std::string path;
		std::string serverProtocol;
		std::string pathInfo; // For Cgi
		std::string pathTranslated;
		std::map< std::string, std::string > headers;
		std::string body;
		std::string GetFileName();
		std::string rawRequestBuffer;
		std::string queryString;
		MatchResult matchResult;
	};

} // namespace http