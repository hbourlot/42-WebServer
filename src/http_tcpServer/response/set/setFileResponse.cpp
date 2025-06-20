#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {
	void TcpServer::setFileResponse(std::string statusCode,
	                                std::string statusMsg,
	                                const std::string &filePath, bool isError) {

		std::string content = readFileContent(filePath);
		if (content.empty()) {
			if (!isError)
				setFileResponse("404", "Not Found", _infos.errorPage[404],
				                true);
			else {
				_response.setResponseError(statusCode, statusMsg);
				_response.setDefaultHeaders(_request);
				setResponse();
			}
			return;
		}

		_response.statusCode = statusCode;
		_response.statusMsg = statusMsg;
		_response.body = content;
		_response.addToHeader("Content-Type", getContentType(filePath));
		_response.setDefaultHeaders(_request);
		setResponse();
	}
} // namespace http