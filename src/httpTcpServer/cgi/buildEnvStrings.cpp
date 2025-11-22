#include "httpTcpServer/Cgi.hpp"
#include <iostream>
#include <map>
#include <string>
#include <utility>

static bool isValidEnv(std::string key) {

	static const char *validCgiVars[] = {"AUTH_TYPE",
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

	
	
	newMap["REQUEST_METHOD"] = _request.method;
	newMap["FILE_NAME"] = _filePath;
	newMap["SCRIPT_NAME"] = _request.path;
	newMap["SERVER_PROTOCOL"] = _request.serverProtocol;
	newMap["SERVER_SOFTWARE"] = "42WebServer/1.0";
	newMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	newMap["PATH_INFO"] = _request.pathInfo;
	newMap["PATH_TRANSLATED"] = _request.pathTranslated;
	// newMap["REMOTE_PORT"] = std::to_string(_clientAddress.sin_port); // ! Function std::to_string not exist!
	newMap["QUERY_STRING"] = _request.queryString;

	for (std::map<std::string, std::string>::const_iterator it =
	         _request.headers.begin();
	     it != _request.headers.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); ++i) {
			if (key[i] == '-')
				key[i] = '_';
			else
				key[i] = toupper(key[i]);
		}
		newMap["HTTP_" + key] = it->second;
	}
	// _envp.clear();
	for (std::map<std::string, std::string>::const_iterator it = newMap.begin();
	     it != newMap.end(); ++it) {
		if (!it->second.empty() && isValidEnv(it->first))
			_envStrings.push_back(it->first + "=" + it->second);
	}
}