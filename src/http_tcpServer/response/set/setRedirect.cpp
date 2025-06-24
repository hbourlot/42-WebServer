#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setRedirect(std::string statusCode, std::string statusMsg,
                                  std::string redirection)
{
	_response.statusCode = statusCode;
	_response.statusMsg = statusMsg;
	_response.addToHeader("Location", redirection);
	_response.setDefaultHeaders(_request);
	setResponse();
}