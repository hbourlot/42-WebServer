#include "Config/Configs.hpp"
#include "Config/Debug.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>

// namespace http {
// 	bool TcpServer::handleGetRequest(const Location &location,
// 	                                 sockaddr_in &clientAddress) {
// 		std::string filePath = getFilePath(_request.path, location);

// 		if (isDirectory(filePath))
// 			return handleDirectoryListing(filePath, location);

// 		if (!std::ifstream(filePath.c_str()).is_open()) {
// 			setFileResponse(HTTP_NOT_FOUND, _serverInfo.errorPage[404], true);
// 			return false;
// 		}

// 		setFileResponse(HTTP_OK, filePath);
// 		return true;
// 	}
// } // namespace http
