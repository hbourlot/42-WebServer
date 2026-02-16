#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

static bool isCgirequest(const http::Request& request, const Location& location) {

	if (!location.cgi_pass.empty())
		return true;

	for (size_t i = 0; i < location.cgi_extension.size(); ++i)
		if (location.cgi_extension[i] == ".*") { // ".cgi" accept any kind of cgi
			return true;
		}

	// Extract file extension from the request path
	std::string path = request.fullPath;
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

void http::Router::launchCgi() {

	// Create and execute CGI
	http::Cgi* cgi = new http::Cgi(_request, _serverConfig, &_client);
	cgi->executeCgi();

	// Store CGI info in client
	_client.setCgiPid(cgi->getPid());
	_client.setCgiOutputFd(cgi->getOutputPipe()[0]);

	// Register CGI in map (takes ownership)
	_eventProcessor.registerCgi(cgi);
	_client.setState(CGI_JUST_STARTED);
}

http::Router::Router(Client& client, ClientEventProcessor& processor)
    : _client(client), _request(client.getRequest()), _response(client.getResponse()),
      _serverConfig(client.getServer().getServerInfo()), _eventProcessor(processor) {
}

void http::Router::process() {
	// 1. Find the best matching location for the request URI
	_request.matchLocation = getMatchLocation(_request.uri, _serverConfig.locations);

	// 2. Check for redirects first
	if (checkRedirects()) {
		return; // A redirect response has been built
	}

	// 3. Check if the request method is allowed
	if (!checkAllowedMethods()) {
		_response.buildErrorResponse(HTTP_FORBID_METHOD, _serverConfig);
		return;
	}

	// 4. Figure out the final file path (handling root, fullPath)
	resolvePath();

	// 5. Execute the request (serve static file or run CGI)
	executeRequest();
}

bool http::Router::checkRedirects() {
	if (!_request.matchLocation)
		return false;

	if (!_request.matchLocation->redirection.empty()) {
		_response.buildRedirect(HTTP_MOVED, _request.matchLocation->redirection);
		return true;
	}
	return false;
}

bool http::Router::checkAllowedMethods() {

	if (!_request.matchLocation) {

		return false;
	}

	const std::vector<std::string>& allowedMethods =
	    _request.fileDirectory ? _request.fileDirectory->methods : _request.matchLocation->methods;

	if (allowedMethods.empty())
		return true;

	for (size_t i = 0; i < allowedMethods.size(); ++i) {

		if ((!_request.matchLocation->cgi_pass.empty() || _request.matchLocation->isCgi()) &&
		    (_request._method == "GET" || _request._method == "POST")) {
			return true;
		}

		if (_request._method == allowedMethods[i])
			return true;
	}

	return false; // Method was not found in the list
}

void http::Router::resolvePath() {

	std::string basePath;

	if (_request.matchLocation && !_request.matchLocation->root.empty()) {
		basePath = _request.matchLocation->root;
	} else {
		basePath = _serverConfig.root;
	}

	_request.fullPath = getFilePath(_request, _serverConfig);


	if (_request.fullPath[_request.fullPath.length() - 1] == '/') {

		std::string indexFile;

		if (_request.matchLocation && !_request.matchLocation->index.empty()) {
			indexFile = _request.matchLocation->index;
		} else if (!_serverConfig.index.empty()) {
			indexFile = _serverConfig.index;
		}

		if (!indexFile.empty()) {
			if (indexFile.rfind("./", 0) == 0) {
				indexFile.erase(0, 2);
			}
		}

		_request.fullPath += indexFile;
	} else if (!_serverConfig.index.empty()) {
		_request.fullPath += _serverConfig.index;
	}

}
void http::Router::executeRequest() {

	bool isCgi = false;
	if (_request.matchLocation && (!_request.matchLocation->cgi_pass.empty() || _request.matchLocation->isCgi()) &&
	    (_request._method == "GET" || _request._method == "POST")) {
		if (_request.matchLocation->isFile())
			isCgi = true;
		else {
			std::cout << "DADASD\n";
			isCgi = isCgirequest(_request, *_request.matchLocation);
		}
	}
	if (isCgi) {
		launchCgi();
	} else {
		if (_request._method == "GET") {
			handleGet();
		} else if (_request._method == "POST") {
			handlePost();
		} else if (_request._method == "DELETE") {
			handleDelete();
		}
	}
}

static bool validateRequestMethod(const http::Request& request, const std::vector<std::string>& methods) {
	if (request._method != "GET" && request._method != "POST" && request._method != "DELETE")
		return false;

	for (size_t i = 0; i < methods.size(); ++i) {
		if (request._method == methods[i])
			return true;
	}
	return false;
}


bool http::Router::routeCgiRequest() {

	http::Request& request = _client.getRequest();
	if (request._method == "GET" || request._method == "POST") {
		launchCgi();
		return false;
	} else {
		_client.getResponse().buildErrorResponse(HTTP_FORBID_METHOD, _serverConfig);
	}
	return true;
}

void http::Router::handleGet() {

	if (isDirectory(_request.fullPath)) {
		handleDirectoryListing();
		return;
	}

	if (!std::ifstream(_request.fullPath.c_str()).is_open()) {
		_response.buildErrorResponse(HTTP_NOT_FOUND, _serverConfig);
		return;
	}

	_response.buildFileResponse(HTTP_OK, _request.fullPath, _serverConfig);
}

void http::Router::handlePost() {
	std::string ContentType;

	if (_request.matchLocation->uploadEnable) {
		UploadManager::handleUpload(*_request.matchLocation, _client, _serverConfig);
		return;
	}
	if (_client.getRequest().body.size() > _request.matchLocation->max_body_size) {
		_client.getResponse().buildErrorResponse(HTTP_PAYLOAD, _serverConfig);
		return;
	}

	_client.getResponse().buildResponse(HTTP_OK, "");
}

void http::Router::handleDelete() {

	struct stat st;

	if (stat(_request.fullPath.c_str(), &st) != 0) {
		_response.buildErrorResponse(HTTP_NOT_FOUND, _serverConfig);
		Logs::log(LOGS_ERROR, "File Not Found");
		return;
	}

	if (isDirectory(_request.fullPath)) {
		_response.buildErrorResponse(HTTP_FORBID, _serverConfig);
		Logs::log(LOGS_ERROR, "Cannot delete because its a folder");
		return;
	}
	if (remove(_request.fullPath.c_str())) {
		_response.buildErrorResponse(HTTP_SERVER_ERR, _serverConfig);
		Logs::log(LOGS_ERROR, "Failed to delete File: " + _request.fullPath);
		return;
	}
	_response.buildResponse(HTTP_NO_CONTENT, "");
	Logs::log(LOGS_ERROR, "File deleted Successfully " + _request.fullPath);
}
