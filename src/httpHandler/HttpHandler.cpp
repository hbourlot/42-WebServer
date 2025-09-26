#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <exception>

static const Location *getMatchLocation(const std::string &path, const std::vector<Location> &locations) {

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i) {

		const std::string &locPath = locations[i].path;

		if (path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > matchLength) {
			matchedLocation = &locations[i];
			matchLength = locPath.size();
		}
	}
	return (matchedLocation);
}

static bool validateRequestMethod(const httpRequest &request, const Location &location) {

	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
		return false;

	for (size_t i = 0; i < location.methods.size(); ++i) {
		if (request.method == location.methods[i])
			return true;
	}
	return false;
}

void http::HttpHandler::handle(ClientManager *object, Client &client, const Server &server) {

	const Location *matchedLocation;
	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	matchedLocation = getMatchLocation(request.path, server.locations);
	
	if (!matchedLocation) {
		response = ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
		// return ;
	}

	if (!matchedLocation->redirection.empty()) {
		response = ResponseBuilder::buildRedirect(HTTP_MOVED, matchedLocation->redirection);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
		// return ;
	}

	if (!validateRequestMethod(request, *matchedLocation)) {
		response = ResponseBuilder::buildFileResponse(HTTP_FORBID_METHOD, DFL_405, server, true);
		client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
		// return ;
	}

	std::cout << "CGI Path" << matchedLocation->path << std::endl;
	std::cout << "CGI EXTENSION" << matchedLocation->cgi_extension.empty() << std::endl;
	if (matchedLocation->cgi_extension.empty() == false){
		std::cout << "Vamos para CGI" << std::endl;
		handleCgi(object, &client, request, *matchedLocation);
	}
	//!!! * Handler CGI
	// try {
	// 	handleCgi(object, &client, request, *matchedLocation);
	// } catch (std::exception &e) {
	// 	std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	// 	exit(0);
	// }

	// std::string filePath = getFilePath(request.path, *matchedLocation);

	// try {
	// 	TcpServer::parseCgi(object, *matchedLocation, filePath, &client);
	// } catch (std::exception &e) {
	// 	std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	// 	// Exit program properly since Allocation failed
	// 	exit(0);
	// }

	if (request.method == "GET")
		return (handleGet(client, server, *matchedLocation));
	else if (request.method == "POST")
		return (handlePost(client, server, *matchedLocation));
	else if (request.method == "DELETE")
		return (handleDelete(client, server, *matchedLocation));
	// return (true);
}

bool http::HttpHandler::handleCgi(ClientManager *clientManager, Client *client, const httpRequest &request,
                                  const Location &location) {

	std::string path;
	std::string filePath;

	path = request.path;
	filePath = getFilePath(path, location);

	std::cout << "Path: " << path << " | Filepath: " << filePath << std::endl; //Path: /cgi-bin/hello.py | Filepath: ./var/www/cgi-bin/hello.py

	if (location.methods.empty()) {
		std::cout << "Invalid methods to execute CGI" << std::endl;
	}

	ICgi *cgi = new PythonCgi(client, clientManager, filePath);
	if (!cgi) {
		std::cerr << "HandleCgi Error: Error allocating memory of Cgi." << std::endl;
	} else {
		client->addCgi(cgi);
		client->executeCgi();
	}
	return true;
}

void http::HttpHandler::handleGet(Client &client, const Server &server, const Location &location) {
	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	std::string filePath = getFilePath(request.path, location);

	if (isDirectory(filePath)) {
		handleDirectoryListing(client, server, filePath, location);
		return;
	}

	if (!std::ifstream(filePath.c_str()).is_open()) {
		std::cout << "[DEBUG] Sending bruh1" << std::endl;
		response = ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));

		// return false;
	}

	response = ResponseBuilder::buildFileResponse(HTTP_OK, filePath, server);
	// client.getWriteBuffer() = ResponseBuilder::buildResponseString(response, request);
	// std::cout << "[DEBUG] Sending 2" << std::endl;
	client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
	// return true;
}

void http::HttpHandler::handlePost(Client &client, const Server &serverInfo, const Location &location) {
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();

	if (!location.cgi_path.empty()) {
		std::cout << "HERE CGI POST" << std::endl;
	}

	// if (request.path == "/login")
	// {
	// 	httpResponse result = validateForm(request);

	// 	if (!result.body.empty())
	// 	{
	// 		// setFileResponse(result.statusCode, result.statusMsg,
	// 		// result.body);
	// 	}
	// 	// else
	// 	// setResponseError(result.statusCode, result.statusMsg);
	// }
	else if (location.uploadEnable) {
		// std::cout << "_request.headers " << _request.headers["Content-Type"] << std::endl;
		// if(_request.headers["Content-Type"] == "")
		// parseMultipart(location);
	} else if (!location.uploadEnable) {
		// setResponseError(HTTP_UPLOAD_FORBID);
		client.getResponse() = ResponseBuilder::buildErrorResponse(HTTP_UPLOAD_FORBID);
			std::cout << "[DEBUG] Sending 3" << std::endl;

		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));

		// return (false);
	} else {
		// setFileResponse(HTTP_NOT_FOUND, serverInfo.errorPage[404], true);
		client.getResponse() =
		    ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, serverInfo.errorPage.at(404), serverInfo, true);
		std::cout << "[DEBUG] Sending 4" << std::endl;
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));

		// return (false);
	}
	// return true;
}
void http::HttpHandler::handleDelete(Client &client, const Server &server, const Location &location) {
	httpResponse &response = client.getResponse();
	httpRequest &request = client.getRequest();

	std::string filePath = getFilePath(request.path, location);

	std::cout << filePath << std::endl;
	if (isDirectory(filePath)) {
		std::cout << "Is a dir cannot delete" << std::endl;
		// return (false);
	}
	if (remove(filePath.c_str()))
		std::cout << "Files not delete" << std::endl;
	// return (true);
}
