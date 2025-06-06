#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http
{
	static std::string buildHttpResponse(const httpResponse &response)
	{
		std::ostringstream responseString;
		responseString << "HTTP/1.1 " << response.statusCode << " "
		               << response.statusMsg << "\r\n";

		{
			//! Agregar headers
		}
		responseString << "\r\n";
		responseString << response.body;

		return responseString.str();
	}

	void TcpServer::setResponse()
	{
		m_serverMessage = buildHttpResponse(response);
	}
	// void TcpServer::setResponse(std::string statusCode, std::string
	// statusMsg,
	//                             std::string contentType, std::string body)
	// {
	// 	//   std::string body = statusMsg + " (" + statusCode + ")";
	// 	std::ostringstream response;
	// 	response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
	// 	         << "Content-Type: " << contentType << "\r\n"
	// 	         << "Content-Length: " << body.length() << "\r\n"
	// 	         << "Connection: close\r\n" // Should be keep-alive because its
	// 	                                    // http 1.1
	// 	         << "\r\n"
	// 	         << body;
	// 	m_serverMessage = response.str();
	// 	// log(response.str());
	// }

	// void TcpServer::setResponseError(std::string statusCode,
	//                                  std::string statusMsg)
	// {
	// 	std::string body = statusMsg + " (" + statusCode + ")";
	// 	setResponse(statusCode, statusMsg, "text/plain", body);
	// }

	// // ! Para adicionar conforme venha no request
	// // if (parsed.headers["Connection"] == "close") {
	// //     response << "Connection: close\r\n";
	// //     keepAlive = false;
	// // } else {
	// //     response << "Connection: keep-alive\r\n";
	// //     keepAlive = true;
	// // }

	// bool TcpServer::setHtmlResponse(std::string statusCode,
	//                                 std::string statusMsg,
	//                                 const std::string &htmlFilePath)
	// {

	// 	std::ifstream htmlFile(htmlFilePath.c_str());
	// 	if (!htmlFile.is_open())
	// 	{
	// 		if (htmlFilePath == DFL_404)
	// 		{
	// 			setResponse("404", "Not Found", "text/plain", "404 Not Found");
	// 			return false;
	// 		}
	// 		return setHtmlResponse("404", "Not Found", DFL_404);
	// 	}
	// 	// Read the HTML file into a string
	// 	std::ostringstream buffer;
	// 	buffer << htmlFile.rdbuf();
	// 	std::string fileContent = buffer.str();
	// 	htmlFile.close();
	// 	std::ostringstream response;
	// 	response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
	// 	         << "Content-type: text/html\r\n"
	// 	         << "Content-Length: " << fileContent.size() << "\r\n"
	// 	         << "Connection: close\r\n" // Should be keep-alive because its
	// 	                                    // http 1.1
	// 	         << "\r\n"
	// 	         << fileContent;

	// 	m_serverMessage = response.str();

	// 	std::string log_str =
	// 	    "HTTP/1.1 " + statusCode + " " + statusMsg + "\r\n";
	// 	log(log_str);
	// 	return (true);
	// }
} // namespace http
