#include "http_tcpServer/Cgi.hpp"

std::string
http::Cgi::getServerName(std::map<std::string, std::string> &headers) const {

	std::string host = headers.count("Host") ? headers.at("Host") : "";
	size_t colon = host.rfind(":");

	return (colon != std::string::npos) ? host.substr(0, colon) : host;
}