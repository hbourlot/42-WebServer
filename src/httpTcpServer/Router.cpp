#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>

static bool isCgirequest(const http::Request &request, const Location &location)
{

	if (!location.cgi_pass.empty())
		return true;
		
	// Extract file extension from the request path
	std::string path = request.getFullPath();
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
	{
		return false; // No extension found
	}

	std::string extension = path.substr(dotPos); // Includes the dot (.py, .cgi, etc.)

	// Check if the extension is in the location's CGI extensions
	for (size_t i = 0; i < location.cgi_extension.size(); ++i) {
		if (location.cgi_extension[i] == extension || location.cgi_extension[i] == ".*") {
			return true;
		}
	}

	return false;
}

void http::Router::launchCgi()
{

	http::Cgi *cgi = new http::Cgi(_request, _serverConfig, &_client);

	if (!cgi)
	{
		_client.getResponse().buildErrorResponse(HTTP_SERVER_ERR, _client.getServer().getServerInfo());
		return;
	}
	if (cgi->executeCgi())
	{
		delete cgi;
		_client.getResponse().buildErrorResponse(HTTP_SERVER_ERR, _client.getServer().getServerInfo());
		return;
	}

	// Store CGI info in client
	_client.setCgiPid(cgi->getPid());
	_client.setCgiOutputFd(cgi->getOutputPipe()[0]);

	// Register CGI in map (takes ownership)
	_eventProcessor.registerCgi(cgi);
	_client.setState(CGI_JUST_STARTED);
}

http::Router::Router(Client& client, EventProcessor& processor)
	: _client(client), _request(client.getRequest()), _response(client.getResponse()),
	  _serverConfig(client.getServer().getServerInfo()), _eventProcessor(processor) {
}

bool http::Router::handleRouteProtected()
{

	std::string uri = _client.getRequest().getUri();
	Session &session = _eventProcessor.getSessionManager().getSession(_client.getSessionID());
	bool isAuthenticated = session.getSessionData("authenticated") == "true";
	const Location *location = _request.getMatchLocation();

	if (isProtectedRoute(uri) && !isAuthenticated)
	{

		std::string port = ft_to_string(_client.getServer().getServerInfo().port[0]); //MUDAR
		std::string path = location ? location->auth_login_page : _client.getServer().getServerInfo().root;
		std::string host = _client.getServer().getServerInfo().host;
		p(host);
		std::string alternativeRoute = ft_to_string("http://" + host + ":" + port);
		alternativeRoute = joinPath(alternativeRoute, path);

		p(alternativeRoute);
		_client.getResponse().buildRedirect(HTTP_TEMP_REDIRECT, alternativeRoute);
		return true;
	}

	return false;
}

void http::Router::process()
{

	if (handleRouteProtected())
		return;

	_request.setMatchLocation(getMatchLocation(_request.getUri(), _serverConfig.locations));

	if (checkRedirects())
	{
		return;
	}
	if (!checkAllowedMethods())
	{
		_response.buildErrorResponse(HTTP_FORBID_METHOD, _serverConfig);
		return;
	}

	resolvePath();
	executeRequest();
}

bool http::Router::isProtectedRoute(const std::string &uri)
{

	// Adding protected routes
	const Location *location = _request.getMatchLocation();
	if (location && location->auth)
	{
		_protectedRoutes.insert(_request.getUri());
	}

	for (std::set<std::string>::const_iterator it = _protectedRoutes.begin(); it != _protectedRoutes.end(); ++it)
	{
		const std::string &prefix = *it;
		if (uri.rfind(prefix, 0) == 0)
			return true;
	}
	return false;
}

bool http::Router::checkRedirects()
{
	if (!_request.getMatchLocation())
		return false;

	if (!_request.getMatchLocation()->redirection.empty())
	{
		_response.buildRedirect(HTTP_MOVED, _request.getMatchLocation()->redirection);
		return true;
	}
	return false;
}

bool http::Router::checkAllowedMethods()
{

	if (!_request.getMatchLocation())
	{

		return false;
	}

	const std::vector<std::string> &allowedMethods = _request.getMatchLocation()->methods;

	const Location *location = _request.getMatchLocation();
	if (location->isCgi())
	{

		bool methodValid = false;
		for (size_t i = 0; i < location->methods.size(); ++i)
			if (location->methods[i] == _client.getRequest().getMethod())
			{
				methodValid = true;
				break;
			}
		if (!methodValid)
			return false;
	}

	if (allowedMethods.empty())
		return true;

	for (size_t i = 0; i < allowedMethods.size(); ++i)
	{

		if (_request.getMethod() == allowedMethods[i])
			return true;
	}
	if (_request.getMatchLocation()->isFile())
	{
		for (size_t i = 0; i < _request.getFileDirectory()->methods.size(); ++i)
		{
			if (_request.getMethod() == _request.getFileDirectory()->methods[i])
				return true;
		}
	}

	return false; // Method was not found in the list
}

void http::Router::resolvePath()
{

	std::string basePath;

	if (_request.getMatchLocation() && !_request.getMatchLocation()->root.empty())
	{
		basePath = _request.getMatchLocation()->root;
	}
	else
	{
		basePath = _serverConfig.root;
	}

	_request.setFullPath(getFilePath(_request, _serverConfig));

	if (_request.getFullPath()[_request.getFullPath().length() - 1] == '/')
	{

		std::string indexFile;

		if (_request.getMatchLocation() && !_request.getMatchLocation()->index.empty())
		{
			indexFile = _request.getMatchLocation()->index;
		}
		else if (!_serverConfig.index.empty())
		{
			indexFile = _serverConfig.index;
		}

		if (!indexFile.empty())
		{
			if (indexFile.rfind("./", 0) == 0)
			{
				indexFile.erase(0, 2);
			}
		}

		_request.getFullPath() += indexFile;
	}
	else if (!_serverConfig.index.empty())
	{
		_request.getFullPath() += _serverConfig.index;
	}
}
void http::Router::executeRequest()
{

	bool isCgi = false;
	if (_request.getMatchLocation() &&
	    (!_request.getMatchLocation()->cgi_pass.empty() || _request.getMatchLocation()->isCgi()) &&
	    (_request.getMethod() == "GET" || _request.getMethod() == "POST"))
	{
		isCgi = isCgirequest(_request, *_request.getMatchLocation());
		if (!isCgi)
		{
			_client.getResponse().buildErrorResponse(HTTP_FORBID, _serverConfig);
			return;
		}
	}
	if (isCgi)
	{
		launchCgi();
	}
	else
	{
		if (_request.getMethod() == "GET")
		{
			handleGet();
		}
		else if (_request.getMethod() == "POST")
		{
			handlePost();
		}
		else if (_request.getMethod() == "DELETE")
		{
			handleDelete();
		}
	}
}

void http::Router::handleGet()
{

	if (isDirectory(_request.getFullPath()))
	{
		handleDirectoryListing();
		return;
	}

	if (!std::ifstream(_request.getFullPath().c_str()).is_open())
	{
		_response.buildErrorResponse(HTTP_NOT_FOUND, _serverConfig);
		return;
	}

	_response.buildFileResponse(HTTP_OK, _request.getFullPath(), _serverConfig);
}

void http::Router::handlePost()
{
	std::string ContentType;
	if (_request.getMatchLocation()->uploadEnable)
	{
		UploadManager::handleUpload(*_request.getMatchLocation(), _client, _serverConfig);
		return;
	}
	if (_client.getRequest().getBody().size() > _request.getMatchLocation()->max_body_size)
	{
		_client.getResponse().buildErrorResponse(HTTP_PAYLOAD, _serverConfig);
		return;
	}

	_client.getResponse().buildResponse(HTTP_OK, "");
}

void http::Router::handleDelete()
{

	struct stat st;

	if (stat(_request.getFullPath().c_str(), &st) != 0)
	{
		_response.buildErrorResponse(HTTP_NOT_FOUND, _serverConfig);
		Logs::log(LOGS_ERROR, "File Not Found");
		return;
	}

	if (isDirectory(_request.getFullPath()))
	{
		_response.buildErrorResponse(HTTP_FORBID, _serverConfig);
		Logs::log(LOGS_ERROR, "Cannot delete because its a folder");
		return;
	}
	if (remove(_request.getFullPath().c_str()))
	{
		_response.buildErrorResponse(HTTP_SERVER_ERR, _serverConfig);
		Logs::log(LOGS_ERROR, "Failed to delete File: " + _request.getFullPath());
		return;
	}
	_response.buildResponse(HTTP_NO_CONTENT, "");
	Logs::log(LOGS_WARN, "File deleted Successfully " + _request.getFullPath());
}
