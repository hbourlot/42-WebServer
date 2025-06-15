#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

	void TcpServer::setResponse(std::string statusCode, std::string statusMsg,
	                            std::string contentType, std::string body,
	                            int len) {
		//   std::string body = statusMsg + " (" + statusCode + ")";
		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
		         << "Content-Type: " << contentType << "\r\n"
		         << "Content-Length: " << len << "\r\n"
		         << "Connection: close\r\n" // Should be keep-alive because its
		                                    // http 1.1
		         << "\r\n"
		         << body;
		m_serverMessage = response.str();
	}
} // namespace http
