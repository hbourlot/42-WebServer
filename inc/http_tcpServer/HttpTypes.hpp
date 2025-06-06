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

	void setDefaultHeaders();
	void setSuccess();
};