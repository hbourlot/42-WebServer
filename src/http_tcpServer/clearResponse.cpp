#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

void http::TcpServer::clearResponse(httpRequest &request,
                                    std::string &serverMessage)
{
	std::cerr << "Enters Here" << __func__ << std::endl;

	request.headers.clear();
	request.method.clear();
	request.body.clear();

	_response.statusCode.clear();
	_response.statusMsg.clear();
	_response.headers.clear();
	_response.body.clear();

	std::cout << "Clear body" << _response.body << std::endl;

	_serverMessage.clear();
}