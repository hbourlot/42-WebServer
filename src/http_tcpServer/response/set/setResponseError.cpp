#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::setResponseError(const HttpStatusCode &status)
{
	std::string body = status.message + " (" + status.code + ")";

	prepareResponse(status, body, "Content-Type", "text/plain");
}