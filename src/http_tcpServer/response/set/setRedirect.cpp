#include "http_tcpServer/Http_tcpServer_linux.hpp"


void http::TcpServer::setRedirect(const HttpStatusCode &status,
                                  std::string redirection)
{
	prepareResponse(status, "", "Location", redirection);
}