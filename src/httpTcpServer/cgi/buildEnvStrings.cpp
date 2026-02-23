#include "httpTcpServer/Cgi.hpp"
#include <iostream>
#include <map>
#include <string>
#include <utility>

static bool isValidEnv(std::string key) {

	static const char* validCgiVars[] = {"AUTH_TYPE",
	                                     "CONTENT_LENGTH",
	                                     "CONTENT_TYPE",
	                                     "DOCUMENT_ROOT",
	                                     "GATEWAY_INTERFACE",
	                                     "HTTPS",
	                                     "PATH_INFO",
	                                     "PATH_TRANSLATED",
	                                     "QUERY_STRING",
	                                     "REMOTE_ADDR",
	                                     "REMOTE_HOST",
	                                     "REMOTE_IDENT",
	                                     "REMOTE_PORT",
	                                     "REMOTE_USER",
	                                     "REQUEST_METHOD",
	                                     "REQUEST_URI",
	                                     "SCRIPT_FILENAME",
	                                     "SCRIPT_NAME",
	                                     "SERVER_NAME",
	                                     "SERVER_PORT",
	                                     "SERVER_PROTOCOL",
	                                     "SERVER_SOFTWARE",
	                                     "HTTP_COOKIE",
	                                     "HTTP_USER_AGENT",
	                                     "HTTP_REFERER",
	                                     "HTTP_ACCEPT",
	                                     "HTTP_ACCEPT_LANGUAGE",
	                                     "HTTP_ACCEPT_ENCODING",
	                                     "HTTP_CONNECTION",
	                                     "HTTP_HOST",
	                                     0};
	for (size_t i = 0; validCgiVars[i]; ++i) {
		if (key == validCgiVars[i])
			return true;
	}

	if (key.size() > 5 && key.substr(0, 5) == "HTTP_")
		return true;

	return false;
}

void http::Cgi::buildEnvStrings() {

	std::map<std::string, std::string> newMap;

	for (std::map<std::string, std::string>::const_iterator it = _request._headers.begin();
	     it != _request._headers.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); ++i) {
			if (key[i] == '-')
				key[i] = '_';
			else
				key[i] = toupper(key[i]);
		}
		newMap["HTTP_" + key] = it->second;
	}

	newMap["REQUEST_METHOD"] = _request._method;
	newMap["FILE_NAME"] = _filePath;

	std::string documentRoot;
	if (_request._matchLocation && _request._matchLocation->isFile()) {
		documentRoot = _request._fileDirectory->root;
	} else if (_request._matchLocation) {
		documentRoot = _request._matchLocation ? _request._matchLocation->root : std::string("");
	}
	if (documentRoot.empty()) {
		documentRoot = _serverInfo.root;
	}
	newMap["DOCUMENT_ROOT"] = documentRoot;

	newMap["SERVER_PROTOCOL"] = _request._serverProtocol;
	newMap["SERVER_SOFTWARE"] = "42WebServer/1.0";
	newMap["GATEWAY_INTERFACE"] = "CGI/1.1";

	newMap["REQUEST_URI"] = _request._uri;
	newMap["SCRIPT_NAME"] = _request._uri;
	newMap["PATH_INFO"] = _request._uri;
	if (_request._headers.count("Content-Length")) {
		newMap["CONTENT_LENGTH"] = _request._headers.at("Content-Length");
	} else {
		newMap["CONTENT_LENGTH"] = ft_to_string(_request._body.size());
	}

	if (_request._headers.count("Content-Type"))
		newMap["CONTENT_TYPE"] = _request._headers.at("Content-Type");

	std::string pathInfoPart = newMap["PATH_INFO"];
	if (_request._headers.count("Content-Length")) {
		newMap["CONTENT_LENGTH"] = _request._headers.at("Content-Length");
	} else if (_request._bodyInDisk) {
		newMap["CONTENT_LENGTH"] = ft_to_string(_request._bodyFdSize);
	} else {
		newMap["CONTENT_LENGTH"] = ft_to_string(_request._body.size());
	}

	// SERVER_NAME and SERVER_PORT from Host header if present, else config
	std::string hostHeader;
	std::string hostName;
	std::string hostPort;
	std::map<std::string, std::string>::const_iterator itHost = _request._headers.find("Host");
	if (itHost != _request._headers.end()) {
		hostHeader = itHost->second;
		size_t colonPos = hostHeader.find(":");
		if (colonPos != std::string::npos) {
			hostName = hostHeader.substr(0, colonPos);
			hostPort = hostHeader.substr(colonPos + 1);
		} else {
			hostName = hostHeader;
			hostPort = ft_to_string(_serverInfo.port); // fallback to configured port
		}
	} else {
		hostName = _serverInfo.host;
		hostPort = ft_to_string(_serverInfo.port);
	}
	newMap["SERVER_NAME"] = hostName;
	newMap["SERVER_PORT"] = hostPort;

	// Remote address
	char addrBuf[INET_ADDRSTRLEN] = {0};
	if (inet_ntop(AF_INET, &_clientAddress.sin_addr, addrBuf, sizeof(addrBuf))) {
		newMap["REMOTE_ADDR"] = addrBuf;
	}
	newMap["REMOTE_PORT"] = ft_to_string(ntohs(_clientAddress.sin_port));

	// Absolute script path
	newMap["SCRIPT_FILENAME"] = _filePath;
	newMap["QUERY_STRING"] = _request._queryString;

	for (std::map<std::string, std::string>::const_iterator it = newMap.begin(); it != newMap.end(); ++it) {
		if (/* !it->second.empty() && */ isValidEnv(it->first))
			_envStrings.push_back(it->first + "=" + it->second);
	}
};

// void http::Cgi::buildEnvStrings() {

// 	std::map< std::string, std::string > newMap;

// 	for (std::map< std::string, std::string >::const_iterator it = _request.headers.begin();
// 	     it != _request.headers.end(); ++it) {
// 		std::string key = it->first;
// 		for (size_t i = 0; i < key.size(); ++i) {
// 			if (key[i] == '-')
// 				key[i] = '_';
// 			else
// 				key[i] = toupper(key[i]);
// 		}
// 		newMap["HTTP_" + key] = it->second;
// 	}

// 	newMap["REQUEST_METHOD"] = _request._method;
// 	newMap["FILE_NAME"] = _filePath;
// 	if (_request.matchResult.location != NULL)
// 		newMap["DOCUMENT_ROOT"] = _request.matchResult.location->root;
// 	else
// 		newMap["DOCUMENT_ROOT"] = _request.matchResult.file->cgi_pass;

// 	newMap["SERVER_PROTOCOL"] = _request.serverProtocol;
// 	newMap["SERVER_SOFTWARE"] = "42WebServer/1.0";
// 	newMap["GATEWAY_INTERFACE"] = "CGI/1.1";

// 	newMap["REQUEST_URI"] = _request.path;
// 	newMap["SCRIPT_NAME"] = _request.path;
// 	newMap["PATH_INFO"] = _request.path;
// 	if (_request.headers.count("Content-Length")) {
// 		newMap["CONTENT_LENGTH"] = _request.headers.at("Content-Length");
// 	} else {
// 		newMap["CONTENT_LENGTH"] = ft_to_string(_request.body.size());
// 	}

// 	if (_request.headers.count("Content-Type"))
// 		newMap["CONTENT_TYPE"] = _request.headers.at("Content-Type");

// 	if (_request.matchResult.location != NULL) {

// 		newMap["PATH_TRANSLATED"] =
// 		    _request.matchResult.location->root + (newMap["PATH_INFO"].empty() ? std::string("/") : _request.pathInfo);
// 	} else
// 		newMap["PATH_TRANSLATED"] =
// 		    _request.matchResult.file->cgi_pass + (newMap["PATH_INFO"].empty() ? std::string("/") : _request.pathInfo);

// 	newMap["REMOTE_PORT"] = ft_to_string(_clientAddress.sin_port);
// 	newMap["QUERY_STRING"] = _request.queryString;

// 	for (std::map< std::string, std::string >::const_iterator it = newMap.begin(); it != newMap.end(); ++it) {
// 		if (/* !it->second.empty() && */ isValidEnv(it->first))
// 			_envStrings.push_back(it->first + "=" + it->second);
// 	}
// };