#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	const std::set<std::string> TcpServer::validCgiExtensions =
	    createValidCgiExtensions();

	bool TcpServer::isValidCgiExtension(const std::string &ext)
	{
		return validCgiExtensions.count(ext) > 0;
	}
} // namespace http