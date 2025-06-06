#include "http_tcpServer/Http_tcpServer_linux.hpp"

bool isDirectory(const std::string &filePath)
{
	struct stat s;
	if (stat(filePath.c_str(), &s) != 0)
		return (false);
	return (S_ISDIR(s.st_mode));
}

std::string getFilePath(std::string &path, const Location *location)
{

	std::string relativePath = path.substr(location->path.length());
	std::string filePath = joinPath(location->root, relativePath);

	return (filePath);
}

std::string joinPath(const std::string &base, const std::string &sub)
{
	if (!base.empty() && (base[base.length() - 1] == '/' || sub[0] == '/'))
		return (base + sub);
	return (base + "/" + sub);
}