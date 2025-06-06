#pragma once
// #include <map>
#include <string>
#include <vector>

struct httpRequest
{
	std::string method;
	std::string path;
	std::string protocol;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> queries;
};

struct httpResponse
{
	std::string statusCode;
	std::string statusMessage;
	std::string htmlFilePath;
	std::map<std::string, std::string> headers;

	void setDefaultHeaders();
	void setSuccess();
};