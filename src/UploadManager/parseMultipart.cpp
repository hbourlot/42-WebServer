#include "httpTcpServer/HttpTcpServerLinux.hpp"

static std::string extractBoundary(http::Request &request) {

	std::string contentType = request.headers["Content-Type"];
	std::string boundaryPrefix = "boundary=";

	size_t pos = contentType.find(boundaryPrefix);
	if (pos == std::string::npos) {
		return ("");
	}
	std::string boundary = "--" + contentType.substr(pos + boundaryPrefix.size());
	return (boundary);
}

static std::string extractFilePart(std::string &body, const std::string &boundary) {

	size_t start = body.find(boundary);

	if (start == std::string::npos)
		return ("");

	start += boundary.length() + 2;
	size_t end = body.find(boundary, start);
	if (end == std::string::npos)
		end = body.size();
	return (body.substr(start, end - start));
}

static bool splitHeadersAndContent(const std::string &filePart, std::string &headers, std::string &content) {

	size_t headerEnd = filePart.find("\r\n\r\n");

	if (headerEnd == std::string::npos)
		return (false);

	headers = filePart.substr(0, headerEnd);
	content = filePart.substr(headerEnd + 4);

	return (true);
}

static std::string extractFilename(const std::string &headers) {

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

static bool saveFile(const std::string &filename, const std::string &content, const Location &matchLocation,
                     const std::string &author) {

	std::string savePath = matchLocation.uploadStore + '/' + author + "-" + filename;

	std::ofstream newfile(savePath.c_str(), std::ios::binary);
	if (!newfile.is_open())
		return (false);

	newfile << content;
	newfile.close();
	return (true);
}

//! Parts to improve after
bool UploadManager::parseMultipart(const Location &matchLocation, Client &client, const ServerConfig &serverInfo) {

	std::string boundary = extractBoundary(client.getRequest());

	if (boundary.empty()) {
		client.getResponse().buildErrorResponse(HTTP_BAD_REQ, serverInfo);
		Logs::log(LOGS_ERROR, "400 Bad Request: No boundary.");

		return (false);
	}

	std::string &body = client.getRequest().readALlBody();
	std::string filePart = extractFilePart(body, boundary);
	if (filePart.empty()) {
		client.getResponse().buildErrorResponse(HTTP_BAD_REQ, serverInfo);
		Logs::log(LOGS_ERROR, "Bad Request: No boundary filePart.");

		return (false);
	}

	std::string headers;
	std::string content;

	if (!splitHeadersAndContent(filePart, headers, content)) {
		client.getResponse().buildErrorResponse(HTTP_BAD_REQ, serverInfo);
		Logs::log(LOGS_ERROR, "Bad Request: Malformed multipart body");

		return (false);
	}

	std::string filename = extractFilename(headers);

	if (filename.empty()) {
		client.getResponse().buildErrorResponse(HTTP_BAD_REQ, serverInfo);
		Logs::log(LOGS_ERROR, "Bad Request: Filename not found.");
		return (false);
	}

	if (!saveFile(filename, content, matchLocation, client.getSessionId())) {
		client.getResponse().buildErrorResponse(HTTP_SERVER_ERR, serverInfo);
		Logs::log(LOGS_ERROR, "Internal Server Error: File not saved.");
		return (false);
	}
	client.getResponse().buildResponse(HTTP_OK, createUploadBody());
	return (true);
}
