#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {
	void TcpServer::setFileResponse(const HttpStatusCode &status,
	                                const std::string &filePath, bool isError) {
		std::string content = readFileContent(filePath);
		if (content.empty()) {
			if (!isError)
				setFileResponse(HTTP_NOT_FOUND, _serverInfo.errorPage[404],
				                true);
			else {
				setResponseError(status);
				// setResponse();
			}
			return;
		}

		prepareResponse(status, content, "Content-Type",
		                getContentType(filePath));
	}
} // namespace http
