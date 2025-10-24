#include "httpTcpServer/HttpTcpServerLinux.hpp"

static const Location *getMatchLocation(const std::string &path, const std::vector<Location> &locations)
{

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{

		const std::string &locPath = locations[i].path;

		if (path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > matchLength)
		{
			matchedLocation = &locations[i];
			matchLength = locPath.size();
		}
	}
	return (matchedLocation);
}

static bool validateRequestMethod(const httpRequest &request, const Location &location)
{

	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
		return false;

	for (size_t i = 0; i < location.methods.size(); ++i)
	{
		if (request.method == location.methods[i])
			return true;
	}
	return false;
}

void HttpHandler::handle(Client &client, const ServerConfig &server)
{
	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	const Location *matchedLocationPtr = getMatchLocation(request.path, server.locations);

	if (!matchedLocationPtr)
	{
		response = ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
	}

	const Location &matchedLocation = *matchedLocationPtr;

	// printLocation(matchedLocation);

	if (!matchedLocation.redirection.empty())
	{
		response = ResponseBuilder::buildRedirect(HTTP_MOVED, matchedLocation.redirection);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
	}

	if (!validateRequestMethod(request, matchedLocation))
	{
		response = ResponseBuilder::buildFileResponse(HTTP_FORBID_METHOD, DFL_405, server, true);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
	}

	if (request.method == "GET")
		return (handleGet(client, server, matchedLocation));
	else if (request.method == "POST")
		return (handlePost(client, server, matchedLocation));
	else if (request.method == "DELETE")
		return (handleDelete(client, server, matchedLocation));
}

void HttpHandler::handleGet(Client &client, const ServerConfig &server, const Location &location)
{
	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	std::string filePath = getFilePath(request.path, location);

	if (isDirectory(filePath))
	{
		handleDirectoryListing(client, server, filePath, location);
		return;
	}

	if (!std::ifstream(filePath.c_str()).is_open())
	{
		response = ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
	}

	response = ResponseBuilder::buildFileResponse(HTTP_OK, filePath, server);
	// client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
	client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
}

void HttpHandler::handlePost(Client &client, const ServerConfig &serverInfo, const Location &location)
{
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();
	std::string ContentType;

	// printLocation(location);
	// 	! "HERE CGI POST"

	//! if (request.path == "/login")
	// {
	// 	httpResponse result = validateForm(request);
	//
	// 	if (!result.body.empty())
	// 	{
	// 		// setFileResponse(result.statusCode, result.statusMsg,
	// 		// result.body);
	// 	}
	// 	// else
	// 	// setResponseError(result.statusCode, result.statusMsg);
	// }

	if (location.uploadEnable)
	{
		UploadManager::handleUpload(location, client, serverInfo);
	}
	else if (!location.uploadEnable)
	{
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_UPLOAD_FORBID);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
	}
	else
	{
		client.getResponse() =
		    ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, serverInfo.errorPage.at(404), serverInfo, true);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
	}
}

void HttpHandler::handleDelete(Client &client, const ServerConfig &server, const Location &location)
{
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();

	std::string filePath = getFilePath(request.path, location);

	std::cout << filePath << std::endl;
	if (isDirectory(filePath))
	{
		std::cout << "Is a dir cannot delete" << std::endl;
		// return (false);
	}
	if (remove(filePath.c_str()))
		std::cout << "Files not delete" << std::endl;
	// return (true);
}

// ! Did i make this?
std::string parseContentType(std::string &contentType)
{
	std::string parsedContentType;

	parsedContentType = contentType.substr(0, contentType.find(';'));

	return (parsedContentType);
}
