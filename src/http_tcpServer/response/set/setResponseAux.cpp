#include "http_tcpServer/Http_tcpServer_linux.hpp"

std::string readFileContent(const std::string &filePath) {
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

static std::string dateString() {
	time_t timestamp;
	time(&timestamp);
	std::string date = ctime(&timestamp);
	if (!date.empty() && date[date.length() - 1] == '\n')
		date.erase(date.length() - 1);
	return (date);
}

void httpResponse::addToHeader(std::string key, std::string value) {
	this->headers[key] = value;
}

void httpResponse::setDefaultHeaders(httpRequest &request) {
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << body.size();
	addToHeader("Content-Length", oss.str());

	std::map<std::string, std::string>::const_iterator it =
	    request.headers.find("Connection");
	addToHeader("Connection",
	            (it != request.headers.end()) ? it->second : "close");
}
