#pragma once
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

struct Location;

struct httpRequest {
	std::string method;
	std::string path;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> rawQueries;
};

struct httpResponse {
	std::string statusCode;
	std::string statusMsg;
	std::string body;
	std::map<std::string, std::string> headers;

	void setDefaultHeaders();
	void setSuccess();
};

struct Cgi {
	// Valid CGI
	static const std::set<std::string> validCgiExtensions;

	// Configuration
	std::string scriptPath;
	std::string scriptName;
	std::map<std::string, std::string> env; // Environment variables

	// Request data
	std::string method;
	std::vector<std::string> queryString;
	std::string requestBody;
	size_t contentLength;
	std::string contentType;

	// Pipe handling
	int inputPipe[2];
	int outputPipe[2];
	pid_t pid;
};