#include "http_tcpServer/Cgi.hpp"
#include <string>

void http::Cgi::buildCgiVars() {

	this->_cgiVars = _request.headers;

	this->_cgiVars["REQUEST_METHOD"] = _request.method;
	this->_cgiVars["FILE_NAME"] = _filePath;
	this->_cgiVars["SCRIPT_NAME"] = _request.path;
	this->_cgiVars["SERVER_PROTOCOL"] = _request.serverProtocol;
	this->_cgiVars["SERVER_SOFTWARE"] = "42WebServer/1.0";
	this->_cgiVars["GETWAY_INTERFACE"] = "CGI/1.1";
	this->_cgiVars["PATH_INFO"] = _request.pathInfo;
	this->_cgiVars["PATH_TRANSLATED"] = _request.pathTranslated;
	this->_cgiVars["REMOTE_PORT"] = std::to_string(_clientAddress.sin_port);
	this->_cgiVars["QUERY_STRING"] = _request.queryString;

	// Add all HTTP headers as HTTP_* variables
	for (std::map<std::string, std::string>::const_iterator it =
	         _request.headers.begin();
	     it != _request.headers.end(); ++it) {
		std::string key = it->first;
		// Convert header name to uppercase and replace '-' with '_'
		for (size_t i = 0; i < key.size(); ++i) {
			if (key[i] == '-')
				key[i] = '_';
			else
				key[i] = toupper(key[i]);
		}
		this->_cgiVars["HTTP_" + key] = it->second;
	}
}