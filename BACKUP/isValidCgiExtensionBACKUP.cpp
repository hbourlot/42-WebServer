#include "http_tcpServer/Cgi.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

const std::set<std::string> http::Cgi::validCgiExtensions =
    createValidCgiExtensions();

bool http::Cgi::isValidCgiExtension(const std::string &ext) {
	return validCgiExtensions.count(ext) > 0;
};