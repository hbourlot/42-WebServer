#include "http_tcpServer/Cgi.hpp"

std::string
http::Cgi::getServerPort(std::map<std::string, std::string> &headers) const {

	std::string serverPort = headers.count("Host") ? headers.at("Host") : "";
	size_t colon = serverPort.rfind(':');

	return (colon != std::string::npos) ? serverPort.substr(colon + 1)
	                                    : serverPort;
}