#include "httpTcpServer/HttpResponse.hpp"

HttpResponse::HttpResponse()
{
}
HttpResponse::HttpResponse(const httpRequest &request) : _protocol(request.serverProtocol)
{
	std::map<std::string, std::string>::const_iterator it = request.headers.find("Connection");

	if (it != request.headers.end())
	{
		std::string val = it->second;
		_connectionType = std::make_pair("Connection", it->second);
	}
	else
	{
		if (_protocol == "HTTP/1.1")
			_connectionType = std::make_pair("Connection", "keep-alive");
		else
			_connectionType = std::make_pair("Connection", "close");
	}
	std::cout << "connetionType" << _connectionType.first << ":" << _connectionType.second << std::endl;
}
HttpResponse::~HttpResponse()
{
}

//! Member Function

void HttpResponse::addToHeader(std::string key, std::string value)
{
	this->_headers[key] = value;
}

void HttpResponse::setDefaultHeaders(httpRequest request)
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << _body.size();
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

void HttpResponse::setDefaultHeaders()
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << _body.size();
	addToHeader("Content-Length", oss.str());

	addToHeader("Connection", "keep-alive");
}

std::string HttpResponse::buildResponseString()
{
	std::ostringstream responseString;
	responseString << _protocol + " " << _statusCode << " " << _statusMsg << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << _body;

	return responseString.str();
}

// Here function

void HttpResponse::buildResponse(const HttpStatusCode &status, const std::string &body, const std::string &headerKey,
                                 const std::string &headerValue, httpRequest *req)
{
	_statusCode = status.code;
	_statusMsg = status.message;
	_body = body;

	if (!headerKey.empty())
		addToHeader(headerKey, headerValue);

	if (req)
		setDefaultHeaders(*req);
	else
		setDefaultHeaders();
}

void HttpResponse::buildErrorResponse(const HttpStatusCode &status)
{
	std::string body = status.message + " (" + status.code + ")";
	return buildResponse(status, body, "Content-Type", "text/plain");
}
void HttpResponse::buildRedirect(const HttpStatusCode &status, const std::string &url)
{
	return buildResponse(status, "", "Location", url);
}

void HttpResponse::buildFileResponse(const HttpStatusCode &status, const std::string &filePath,
                                     const ServerConfig &server, bool isError)
{
	std::string content = readFileContent(filePath);
	if (content.empty())
	{
		if (!isError)
			return buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		else
			return buildErrorResponse(status);
	}
	return buildResponse(status, content, "Content-Type", getContentType(filePath));
}

std::string HttpResponse::readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

std::string HttpResponse::getContentType(const std::string &filePath)
{
	size_t dot = filePath.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream"; // binario genérico

	std::string ext = filePath.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "txt")
		return "text/plain";
	if (ext == "pdf")
		return "application/pdf";
	return "application/octet-stream";
}
