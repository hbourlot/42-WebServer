#include "httpTcpServer/HttpTcpServerLinux.hpp"

std::string readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

void httpResponse::addToHeader(std::string key, std::string value)
{
	this->headers[key] = value;
}

void httpResponse::setDefaultHeaders(httpRequest request)
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << body.size();
	addToHeader("Content-Length", oss.str());

	std::map<std::string, std::string>::const_iterator it =
	    request.headers.find("Connection");
	addToHeader("Connection",
	            (it != request.headers.end()) ? it->second : "close");
}

void httpResponse::setDefaultHeaders()
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << body.size();
	addToHeader("Content-Length", oss.str());

	addToHeader("Connection", "close");
}
