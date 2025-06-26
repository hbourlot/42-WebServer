#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setBodyResponse(const HttpStatusCode &status,
                                      const std::string &body,
                                      const std::string &contentType)
{
	prepareResponse(status, body, "Content-Type", contentType);
}