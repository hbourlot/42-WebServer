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
};

struct httpResponse
{
	std::string statusCode;
	std::string statusMsg;
	std::string body;
	std::map<std::string, std::string> headers;

	void setDefaultHeaders(httpRequest &request);
	void addHeader(std::string key, std::string value);
	void setResponseError(std::string statusCode, std::string statusMsg);
};