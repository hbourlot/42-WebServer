#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::addCgi(const Location &loc, const httpRequest &request)
{

	Location &fooLoc = const_cast<Location &>(loc);
	httpRequest &fooRequest = const_cast<httpRequest &>(request);

	CgiHandler foo(fooLoc, fooRequest);
	CGI.push_back(foo);
}