#include "CGI/CgiHandler.hpp"
#include "http_tcpServer/Http_tcpServer_Linux.hpp"

bool http::CgiHandler::parseCgi(httpRequest request) {

	if (request.method.empty() || m_scriptName.empty()) {
		return false;
	}

	m_method = request.method;
	m_queryString = request.query;
	m_requestBody = request.body;
	m_contentLength = request.body.length();
	m_contentType = request.headers["Content-Type"];

	return true;
};
