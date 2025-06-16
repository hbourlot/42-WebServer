#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{
	bool TcpServer::handleGetRequest(const Location &location)
	{
		std::string filePath = getFilePath(request.path, location);

		if (isDirectory(filePath))
			return handleDirectoryListing(filePath, location);

		if (!std::ifstream(filePath.c_str()).is_open())
		{
			setFileResponse("404", "Not Found", infos.errorPage[404], true);
			return false;
		}

		setFileResponse("200", "OK", filePath);
		return true;
	}
} // namespace http
