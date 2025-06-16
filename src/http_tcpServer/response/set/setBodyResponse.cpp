#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setBodyResponse(const std::string &statusCode,
                                      const std::string &statusMsg,
                                      const std::string &body,
                                      const std::string &contentType)
{
	response.statusCode = statusCode;
	response.statusMsg = statusMsg;
	response.body = body;
	response.addHeader("Content-Type", contentType);
	response.setDefaultHeaders(request);
	setResponse();
}