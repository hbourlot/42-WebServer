#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::setResponseError(std::string statusCode,
	                                 std::string statusMsg) {
		std::string body = statusMsg + " (" + statusCode + ")";
		setResponse(statusCode, statusMsg, "text/plain", body, body.length());
	}

} // namespace http