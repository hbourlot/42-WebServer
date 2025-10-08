#include "httpTcpServer/Cgi.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

const std::set<std::string> http::Cgi::validCgiExtensions =
    createValidCgiExtensions();

bool http::Cgi::isValidCgiExtension(const std::string &ext) {
	return validCgiExtensions.count(ext) > 0;
};