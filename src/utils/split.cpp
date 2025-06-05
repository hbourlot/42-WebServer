#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, char delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream iss(s);
	std::string token;

	while (std::getline(iss, token, delimiter))
		tokens.push_back(token);
	return tokens;
}
