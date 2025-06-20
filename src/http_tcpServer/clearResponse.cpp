#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::clearResponse(httpRequest &request,
                                    std::string &serverMessage) {
	request.headers.clear();
	request.method.clear();
	request.body.clear();
	_serverMessage.clear();
}