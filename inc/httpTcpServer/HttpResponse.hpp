#pragma once
#include "Config/Configs.hpp"
#include "httpTcpServer/HttpStructs.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

struct httpRequest;

class HttpResponse
{
  private:
	std::string _protocol;
	std::pair<std::string, std::string> _connectionType;
	std::string _statusCode;
	std::string _statusMsg;
	std::string _body;
	std::map<std::string, std::string> _headers;

	void addToHeader(std::string key, std::string value);

  public:
	HttpResponse();
	HttpResponse(const httpRequest &request);
	~HttpResponse();

	// function Member
	std::string buildResponseString();
	void setDefaultHeaders();
	std::string getContentType(const std::string &filePath);
	std::string readFileContent(const std::string &filePath);

	void buildResponse(const HttpStatusCode &status, const std::string &body);
	void buildErrorResponse(const HttpStatusCode &status);
	void buildRedirect(const HttpStatusCode &status, const std::string &url);
	void buildFileResponse(const HttpStatusCode &status, const std::string &filePath, const ServerConfig &server,
	                       bool isError = false);
};

// struct httpResponse
// {
// 	std::string statusCode;
// 	std::string statusMsg;
// 	std::string body;
// 	std::map<std::string, std::string> headers;

// 	void setDefaultHeaders();
// 	void setDefaultHeaders(httpRequest request);
// 	void addToHeader(std::string key, std::string value);
// 	std::string buildResponseString(const httpRequest &request);
// };