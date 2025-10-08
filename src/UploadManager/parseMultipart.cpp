#include "httpTcpServer/HttpTcpServerLinux.hpp"

static std::string extractBoundary(httpRequest &request)
{

	std::string contentType = request.headers["Content-Type"];
	std::string boundaryPrefix = "boundary=";

	size_t pos = contentType.find(boundaryPrefix);
	if (pos == std::string::npos)
	{
		return ("");
	}
	std::string boundary = "--" + contentType.substr(pos + boundaryPrefix.size());
	return (boundary);
}

static std::string extractFilePart(httpRequest &request, const std::string &boundary)
{

	std::string body = request.body;
	size_t start = body.find(boundary);

	if (start == std::string::npos)
		return ("");

	start += boundary.length() + 2;
	size_t end = body.find(boundary, start);

	if (end == std::string::npos)
		end = body.size();

	return (body.substr(start, end - start));
}

static bool splitHeadersAndContent(const std::string &filePart, std::string &headers, std::string &content)
{

	size_t headerEnd = filePart.find("\r\n\r\n");

	if (headerEnd == std::string::npos)
		return (false);

	headers = filePart.substr(0, headerEnd);
	content = filePart.substr(headerEnd + 4);

	return (true);
}

static std::string extractFilename(const std::string &headers)
{

	std::string token = "filename=\"";
	size_t start = headers.find(token);

	if (start == std::string::npos)
		return ("");

	start += token.length();
	size_t end = headers.find("\"", start);

	if (end == std::string::npos)
		return ("");

	return (headers.substr(start, end - start));
}

static bool saveFile(const std::string &filename, const std::string &content, const Location &location)
{

	std::string savePath = location.uploadStore + '/' + filename;

	std::ofstream newfile(savePath.c_str(), std::ios::binary);
	if (!newfile.is_open())
		return (false);

	newfile << content;
	newfile.close();
	return (true);
}

//! Parts to imporve after
bool UploadManager::parseMultipart(const Location &location, Client &client)
{
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();

	std::string boundary = extractBoundary(client.getRequest());

	if (boundary.empty())
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		log("400 Bad Request: No boundary");

		return (false);
	}

	std::string filePart = extractFilePart(client.getRequest(), boundary);

	if (filePart.empty())
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		log("Bad Request: No boundary filePart");

		return (false);
	}

	std::string headers;
	std::string content;

	if (!splitHeadersAndContent(filePart, headers, content))
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		log("Bad Request: Malformed multipart body");

		return (false);
	}

	std::string filename = extractFilename(headers);

	if (filename.empty())
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_BAD_REQ);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		log("Bad Request: Filename not found");
		return (false);
	}

	if (!saveFile(filename, content, location))
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_SERVER_ERR);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		log("Internal Server Error: File not saved");
		return (false);
	}
	std::string msg = "File '" + filename + "' received";
	client.getResponse() = ResponseBuilder::buildResponse(HTTP_OK, msg, "", "", &request);
	client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
	return (true);
}
