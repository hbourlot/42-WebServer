#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

	void TcpServer::setResponseError(std::string statusCode,
									 std::string statusMsg) {
		std::string body = statusMsg + " (" + statusCode + ")";
		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
				 << "Content-Type: text/plain\r\n"
				 << "Content-Length: " << body.length() << "\r\n"
				 << "Connection: close\r\n"
				 << "\r\n"
				 << body;
		m_serverMessage = response.str();
		log(response.str());
	}

	void TcpServer::setHtmlResponse(std::ifstream &htmlFile) {
		// Read the HTML file into a string
		std::ostringstream buffer;
		buffer << htmlFile.rdbuf();
		std::string fileContent = buffer.str();
		htmlFile.close();

		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n"
				 << "Content-type: text/html\r\n"
				 << "Content-Length: " << fileContent.size() << "\r\n"
				 << "Connection: keep-alive\r\n"
				 << "\r\n"
				 << fileContent;

		m_serverMessage = response.str();
	}
} // namespace http
