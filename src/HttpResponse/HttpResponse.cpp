#include "httpTcpServer/HttpResponse.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

	std::map<std::string, std::string>::const_iterator itRange = request.headers.find("Range");

	if (itRange != request.headers.end())
		_range = std::make_pair(itRange->first, itRange->second);
}
HttpResponse::~HttpResponse()
{
}

//! Member Function

void HttpResponse::addToHeader(std::string key, std::string value)
{
	this->_headers[key] = value;
}

void HttpResponse::setDefaultHeaders()
{
	addToHeader("Date", dateString());

	std::ostringstream oss;
	oss << _body.size();
	addToHeader("Content-Length", oss.str());

	addToHeader(_connectionType.first, _connectionType.second);
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

void HttpResponse::buildResponse(const HttpStatusCode &status, const std::string &body)
{
	_statusCode = status.code;
	_statusMsg = status.message;
	_body = body;

	setDefaultHeaders();
}
static std::string createErrorBody(const HttpStatusCode &status)
{
	std::string html;

	html += "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n<title>Error Occurred</title>\n";
	html += "<style>\n";
	html +=
	    "body { font-family: Arial, sans-serif; text-align: center; padding-top: 10%; background-color: #f9f9f9; }\n";
	html += ".error-box { display: inline-block; border-radius: 10px; padding: 2em 3em; background: #fff; box-shadow: "
	        "0 2px 8px rgba(0,0,0,0.1); }\n";
	html += "h1 { margin-bottom: 0.5em; color: #d9534f; }\n";
	html += "p { color: #555; margin-bottom: 1.5em; }\n";
	html += "a.button { text-decoration: none; color: white; background: #007bff; padding: 0.7em 1.5em; border-radius: "
	        "5px; font-weight: bold; }\n";
	html += "a.button:hover { background: #0056b3; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<div class=\"error-box\">";
	html += "<h1>Error " + status.code + "</h1>\n";
	html += "<p>Sorry, " + status.message + "</p>\n";
	html += "<a href=\"/\" class=\"button\">Go Home</a>\n";
	html += "</div>\n";
	html += "</body>\n</html>\n";

	return html;
}

void HttpResponse::buildErrorResponse(const HttpStatusCode &status, const ServerConfig &server)
{
	std::map<int, std::string>::const_iterator it;
	it = server.errorPage.find(atoi(status.code.c_str()));
	if (it != server.errorPage.end())
	{
		std::ifstream file(it->second.c_str());
		if (file.good())
		{
			buildFileResponse(status, it->second, server);
			return;
		}
	}
	// std::string body = status.message + " (" + status.code + ")";
	buildResponse(status, createErrorBody(status));
	addToHeader("Content-Type", "text/html");
}
void HttpResponse::buildRedirect(const HttpStatusCode &status, const std::string &url)
{
	buildResponse(status, "");
	addToHeader("Location", url);
}

static bool parseRange(std::string &rangeValue, off_t &fileSize, off_t &start, off_t &end)
{
	std::string prefix = "bytes=";
	if (rangeValue.compare(0, prefix.size(), prefix) != 0)
		return (false);

	std::string rangeSpec = rangeValue.substr(prefix.size());
	size_t dash = rangeSpec.find('-');
	if (dash == std::string::npos)
		return (false);

	std::string startStr = rangeSpec.substr(0, dash);
	std::string endStr = rangeSpec.substr(dash + 1);

	start = 0;
	end = fileSize - 1;

	if (!startStr.empty())
		start = static_cast<off_t>(atoll(startStr.c_str()));
	if (!endStr.empty())
		end = static_cast<off_t>(atoll(endStr.c_str()));

	if (start >= fileSize)
		return (false);
	if (end >= fileSize)
		end = fileSize - 1;
	if (start > end)
		return (false);

	return (true);
}

void HttpResponse::buildRangeResponse(const std::string &filePath, const ServerConfig &server, struct stat &st)
{
	off_t start;
	off_t end;

	if (!parseRange(_range.second, st.st_size, start, end))
		return (buildErrorResponse(HTTP_RANGE_NOT_SATISFIABLE, server));

	off_t diff = end - start + 1;

	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
		return (buildErrorResponse(HTTP_SERVER_ERR, server));

	file.seekg(start, std::ios::beg);

	std::ostringstream body;
	char buffer[CHUNK_SIZE + 1] = {0};
	while (diff > 0 && file.good())
	{
		std::streamsize toRead = std::min<off_t>(diff, CHUNK_SIZE);
		file.read(buffer, toRead);
		std::streamsize bytesRead = file.gcount();
		if (bytesRead <= 0)
			break;

		body.write(buffer, bytesRead);
		diff -= bytesRead;
	}
	file.close();

	buildResponse(HTTP_PARTIAL_CONTENT, body.str());
	std::ostringstream rangeHeader;
	rangeHeader << "bytes " << start << "-" << end << "/" << st.st_size;
	if (DEBUG)
		Logs::log(INFO, "Request for range: " + rangeHeader.str());
	addToHeader("Content-Range", rangeHeader.str());
	addToHeader("Content-Type", getContentType(filePath));
}
void HttpResponse::buildFileResponse(const HttpStatusCode &status, const std::string &filePath,
                                     const ServerConfig &server)
{
	struct stat st;
	if (stat(filePath.c_str(), &st) != 0)
		return (buildErrorResponse(HTTP_NOT_FOUND, server));

	if (!_range.first.empty())
		return (buildRangeResponse(filePath, server, st));

	std::string content = readFileContent(filePath);

	buildResponse(status, content);
	addToHeader("Content-Type", getContentType(filePath));
}

std::string HttpResponse::readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());

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
