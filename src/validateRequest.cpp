#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

	bool TcpServer::validateRequest(const std::string &requestPath) {

		std::string fullPath = "./content/" + requestPath;
		std::ifstream htmlFile(fullPath.c_str());
		if (!htmlFile.is_open()) {
			log("Error: could not open HTML file");
			log("HTTP/1.1 500 Internal Server Error\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 21\r\n"
				"Connection: close\r\n"
				"\r\n"
				"Internal Server Error");
			return false; // TODO: throw error would be better
		}

		// Read the HTML file into a string
		std::ostringstream buffer;
		buffer << htmlFile.rdbuf();
		std::string fileContent = buffer.str();
		htmlFile.close();

		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n"
				 << "Content-type: text/html\r\n"
				 << "Content-Length: " << fileContent.size() << "\r\n"
				 << "Connection: close\r\n"
				 << "\r\n"
				 << fileContent;

		m_serverMessage = response.str();

		return true;
	}

} // namespace http
