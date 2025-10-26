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

	std::string connectionValue;
	std::map<std::string, std::string>::const_iterator it = request.headers.find("Connection");
	if (it != request.headers.end())
		connectionValue = it->second;
	else
	{
		if (request.serverProtocol == "HTTP/1.1")
			connectionValue = "keep-alive";
		else
			connectionValue = "close";
	}
	addToHeader("Connection", connectionValue);
}

void httpResponse::setDefaultHeaders()
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << body.size();
	addToHeader("Content-Length", oss.str());

	addToHeader("Connection", "keep-alive");
}

std::string httpResponse::buildResponseString(const httpRequest &request)
{
	std::ostringstream responseString;
	responseString << request.serverProtocol + " " << this->statusCode << " " << this->statusMsg << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = this->headers.begin(); it != this->headers.end(); ++it)
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << this->body;

	return responseString.str();
}
//! De momento ponerlo aqui, no encaja del todo en otros archivos

bool httpRequest::shouldCloseConnection()
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Connection");

	if (it != headers.end())
	{
		std::string val = it->second;

		for (std::string::size_type i = 0; i < val.size(); ++i)
			val[i] = std::tolower(val[i]);

		return (val == "close");
	}
	else
	{
		if (serverProtocol == "HTTP/1.1")
			return (false);
		else
			return (true);
	}
}