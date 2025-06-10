#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

static std::string readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

static std::string buildResponse(const httpResponse &response)
{
	std::ostringstream responseString;
	responseString << "HTTP/1.1 " << response.statusCode << " "
	               << response.statusMsg << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = response.headers.begin(); it != response.headers.end(); ++it)
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << response.body;

	return responseString.str();
}

static std::string dateString()
{
	time_t timestamp;
	time(&timestamp);
	std::string date = ctime(&timestamp);
	if (!date.empty() && date[date.length() - 1] == '\n')
		date.erase(date.length() - 1);
	return (date);
}

void httpResponse::addHeader(std::string key, std::string value)
{
	this->headers[key] = value;
}

void httpResponse::setDefaultHeaders(httpRequest &request)
{
	addHeader("Date", dateString());

	std::ostringstream oss;
	oss << body.size();
	addHeader("Content-Length", oss.str());

	std::map<std::string, std::string>::const_iterator it =
	    request.headers.find("Connection");
	addHeader("Connection",
	          (it != request.headers.end()) ? it->second : "close");
}

void httpResponse::setResponseError(std::string statusCode,
                                    std::string statusMsg)
{
	this->statusCode = statusCode;
	this->statusMsg = statusMsg;

	addHeader("Content-Type", "text/plain");

	this->body = statusMsg + " (" + statusCode + ")";
}

namespace http
{

	void TcpServer::setResponse()
	{
		log(m_serverMessage);
		std::cout << "Here" << std::endl;
		m_serverMessage = buildResponse(response);
		std::cout << m_serverMessage << std::endl;
	}

	void TcpServer::setFileResponse(std::string statusCode,
	                                std::string statusMsg,
	                                const std::string &filePath, bool isError)
	{

		std::string content = readFileContent(filePath);
		if (content.empty())
		{
			if (!isError)
				setFileResponse("404", "Not Found", infos.errorPage[404], true);
			else
			{
				response.setResponseError(statusCode, statusMsg);
				setResponse();
			}
			return;
		}

		response.statusCode = statusCode;
		response.statusMsg = statusMsg;
		response.body = content;
		response.addHeader("Content-Type", getContentType(filePath));
		setResponse();
	}

} // namespace http
