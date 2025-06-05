#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{
	bool TcpServer::handleDeleteRequest(const Location *location)
	{
		std::string filePath = getFilePath(request.path, location);

		std::cout << filePath << std::endl;
		if (isDirectory(filePath))
		{
			std::cout << "Is a dir cannot delete" << std::endl;
			return (false);
		}
		if (remove(filePath.c_str()))
			std::cout << "Files not delete" << std::endl;
		return (true);
	}
} // namespace http