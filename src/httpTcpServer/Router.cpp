#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

static bool validateRequestMethod(const http::Request &request, const std::vector<std::string> &methods) {
	if (request._method != "GET" && request._method != "POST" && request._method != "DELETE")
		return false;

	for (size_t i = 0; i < methods.size(); ++i) {
		if (request._method == methods[i])
			return true;
	}
	return false;
}
static std::vector<std::string> resolveMethods(const Directory *loc, const File *file) {
	if (loc && !loc->methods.empty())
		return loc->methods;
	if (file)
		return file->methods;
	return std::vector<std::string>();
}

static bool isCgirequest(const http::Request &request, const Location &location) {

	for (size_t i = 0; i < location.cgi_extension.size(); ++i)
		if (location.cgi_extension[i] == ".*") { // ".cgi" accept any kind of cgi
			return true;
		}

	// Extract file extension from the request path
	std::string path = request.path;
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos) {
		return false; // No extension found
	}

	std::string extension = path.substr(dotPos); // Includes the dot (.py, .cgi, etc.)

	// Check if the extension is in the location's CGI extensions
	for (size_t i = 0; i < location.cgi_extension.size(); ++i) {
		if (location.cgi_extension[i] == extension) {
			return true;
		}
	}

	return false;
}

// priority
// 1 File (se tiver CGI)
// 2 Location
//		- 2.1 RedirectME
//		- 2.2 VALID CGI
// File (se nao tiver CGI)
VALIDATION_STATUS http::Router::validateRequest(Client &client, const ServerConfig &server) {

	http::Request &request = client.getRequest();

	const Location *matchLocation = request.matchLocation;

	if (matchLocation && matchLocation->type == LOCATION_FILE) {
		if (!matchLocation->cgi_pass.empty() && validateRequestMethod(request, matchLocation->methods))
			return VALID_IS_CGI;
	}

	if (matchLocation && matchLocation->type == LOCATION_REDIRECT)
		return VALID_REDIRECT_REQUIRED;

	if (matchLocation && matchLocation->type != LOCATION_FILE) {
		if (!validateRequestMethod(request, matchLocation->methods)) {
			return VALID_METHOD_NOT_ALLOWED;
		}
	}

	if (matchLocation && matchLocation->type == LOCATION_CGI) {
		if (isCgirequest(request, *request.matchLocation))
			return VALID_IS_CGI;
		else
			return VALID_FORBIDDEN;
	}
	return VALID_OK;
}

bool http::Router::routeCgiRequest(Client &client, const ServerConfig &server, const Location &matchLocation,
                                   ClientEventProcessor &processor) {

	http::Request &request = client.getRequest();
	if (request._method == "GET" || request._method == "POST") {
		launchCgi(client, server, matchLocation, processor);
		return false;
	} else {
		client.getResponse().buildErrorResponse(HTTP_FORBID_METHOD, server);
	}
	return true;
}

void http::Router::launchCgi(Client &client, const ServerConfig &server, const Location &matchLocation,
                             ClientEventProcessor &processor) {
	http::Request &request = client.getRequest();

	// Create and execute CGI
	http::Cgi *cgi = new http::Cgi(request, matchLocation.cgi_pass, server, &client);
	cgi->executeCgi();

	// Store CGI info in client
	client.setCgiPid(cgi->getPid());
	client.setCgiOutputFd(cgi->getOutputPipe()[0]);

	// Register CGI in map (takes ownership)
	processor.registerCgi(cgi);
	client.setState(CGI_JUST_STARTED);
}

void http::Router::routeStaticRequest(Client &client, const ServerConfig &server, const Location &matchLocation) {

	http::Request &request = client.getRequest();

	if (request._method == "GET")
		return (handleGet(client, server, matchLocation));
	else if (request._method == "POST")
		return (handlePost(client, server, matchLocation));
	else if (request._method == "DELETE")
		return (handleDelete(client, server, matchLocation));
	else
		client.getResponse().buildErrorResponse(HTTP_FORBID_METHOD, server);
}

void http::Router::handleGet(Client &client, const ServerConfig &server, const Location &matchLocation) {

	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	std::string filePath;

	filePath = getFilePath(request, server);
	std::cout << "filePath: " << filePath << std::endl;
	if (isDirectory(filePath)) {
		handleDirectoryListing(client, server, filePath, matchLocation);
		return;
	}

	if (!std::ifstream(filePath.c_str()).is_open()) {
		response.buildErrorResponse(HTTP_NOT_FOUND, server);
		return;
	}

	response.buildFileResponse(HTTP_OK, filePath, server);
}

void http::Router::handlePost(Client &client, const ServerConfig &serverInfo, const Location &matchLocation) {
	std::string ContentType;

	if (matchLocation.uploadEnable) {
		UploadManager::handleUpload(matchLocation, client, serverInfo);
		return;
	}
	if (client.getRequest().body.size() > matchLocation.max_body_size) {
		client.getResponse().buildErrorResponse(HTTP_PAYLOAD, serverInfo);
		return;
	}

	client.getResponse().buildResponse(HTTP_OK, "");
}

void http::Router::handleDelete(Client &client, const ServerConfig &server, const Location &matchLocation) {
	http::Response &response = client.getResponse();
	http::Request &request = client.getRequest();

	std::string filePath = getFilePath(request, server);

	struct stat st;

	if (stat(filePath.c_str(), &st) != 0) {
		response.buildErrorResponse(HTTP_NOT_FOUND, server);
		Logs::log(LOGS_ERROR, "File Not Found");
		return;
	}

	if (isDirectory(filePath)) {
		response.buildErrorResponse(HTTP_FORBID, server);
		Logs::log(LOGS_ERROR, "Cannot delete because its a folder");
		return;
	}
	if (remove(filePath.c_str())) {
		response.buildErrorResponse(HTTP_SERVER_ERR, server);
		Logs::log(LOGS_ERROR, "Failed to delete File: " + filePath);
		return;
	}
	response.buildResponse(HTTP_NO_CONTENT, "");
	Logs::log(LOGS_ERROR, "File deleted Successfully " + filePath);
}
