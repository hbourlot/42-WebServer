#include "http_tcpServer/Http_tcpServer_linux.hpp"

bool isDirectory(const std::string &filePath)
{
	struct stat s;
	if (stat(filePath.c_str(), &s) != 0)
		return (false);
	return (S_ISDIR(s.st_mode));
}

std::string getFilePath(std::string &path, const Location &location)
{

	std::string relativePath = path.substr(location.path.length());
	std::string filePath = joinPath(location.root, relativePath);

	return (filePath);
}

std::string joinPath(const std::string &base, const std::string &sub)
{
	if (!base.empty() && (base[base.length() - 1] == '/' || sub[0] == '/'))
		return (base + sub);
	return (base + "/" + sub);
}

std::string getContentType(const std::string &path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream"; // binario genérico

	std::string ext = path.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "txt")
		return "text/plain";
	if (ext == "pdf")
		return "application/pdf";
	return "application/octet-stream";
}