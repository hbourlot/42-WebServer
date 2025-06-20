#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setBodyResponse(const std::string &statusCode,
                                      const std::string &statusMsg,
                                      const std::string &body,
                                      const std::string &contentType) {
	_response.statusCode = statusCode;
	_response.statusMsg = statusMsg;
	_response.body = body;
	_response.addToHeader("Content-Type", contentType);
	_response.setDefaultHeaders(_request);
	setResponse();
}