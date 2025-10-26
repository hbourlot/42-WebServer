#include "httpTcpServer/HttpTcpServerLinux.hpp"

httpResponse ResponseBuilder::buildResponse(const HttpStatusCode &status, const std::string &body,
                                            const std::string &headerKey, const std::string &headerValue,
                                            httpRequest *req)
{
	httpResponse res;
	res.statusCode = status.code;
	res.statusMsg = status.message;
	res.body = body;

	if (!headerKey.empty())
		res.addToHeader(headerKey, headerValue);

	if (req)
		res.setDefaultHeaders(*req);
	else
		res.setDefaultHeaders();

	return res;
}

httpResponse ResponseBuilder::buildErrorResponse(const HttpStatusCode &status)
{
	std::string body = status.message + " (" + status.code + ")";
	return buildResponse(status, body, "Content-Type", "text/plain");
}
httpResponse ResponseBuilder::buildRedirect(const HttpStatusCode &status, const std::string &url)
{
	return buildResponse(status, "", "Location", url);
}
httpResponse ResponseBuilder::buildFileResponse(const HttpStatusCode &status, const std::string &filePath,
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

std::string ResponseBuilder::readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}
std::string ResponseBuilder::getContentType(const std::string &filePath)
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
