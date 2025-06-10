#pragma once
#include <string>
#include <vector>

struct httpRequest
{
	std::string method;
	std::string path;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> rawQueries;
	std::map<std::string, std::string> queryParams;
	// ! map queryParams maybe for an future use on CGI
};

struct httpResponse
{
	std::string statusCode;
	std::string statusMsg;
	std::string body;
	std::map<std::string, std::string> headers;

	void setDefaultHeaders(httpRequest &request);
	void addHeader(std::string key, std::string value);
	void setResponse(std::string statusCode, std::string statusMsg,
	                 std::string contentType, std::string body);
	void setResponseError(std::string statusCode, std::string statusMsg);
	void setHtmlResponse(std::string statusCode, std::string statusMsg,
	                     const std::string &htmlFilePath, bool isError);
	void setHtmlErrorResponse(std::string statusCode, std::string statusMsg,
	                          const std::string &htmlFilePath);
	void setSuccess();
};