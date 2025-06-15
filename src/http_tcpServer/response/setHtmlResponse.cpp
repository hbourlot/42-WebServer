#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {

	bool TcpServer::setHtmlResponse(std::string statusCode,
	                                std::string statusMsg,
	                                const std::string &htmlFilePath) {

		std::ifstream htmlFile(htmlFilePath.c_str());
		if (!htmlFile.is_open()) {
			if (htmlFilePath == DFL_404) {

				std::string errorMessage = "404 Not Found";
				setResponse("404", "Not Found", "text/plain", errorMessage,
				            errorMessage.length());
				return false;
			}
			return setHtmlResponse("404", "Not Found", DFL_404);
		}
		// Read the HTML file into a string
		std::ostringstream buffer;
		buffer << htmlFile.rdbuf();
		std::string fileContent = buffer.str();
		htmlFile.close();
		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
		         << "Content-type: text/html\r\n"
		         << "Content-Length: " << fileContent.size() << "\r\n"
		         << "Connection: close\r\n" // Should be keep-alive because its
		                                    // http 1.1
		         << "\r\n"
		         << fileContent;

		m_serverMessage = response.str();

		std::string log_str =
		    "HTTP/1.1 " + statusCode + " " + statusMsg + "\r\n";
		log(log_str);
		return (true);
	}
} // namespace http