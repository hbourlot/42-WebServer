#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

static std::string buildResponse(const httpResponse &response)
{
	std::ostringstream responseString;
	responseString << "HTTP/1.1 " << response.statusCode << " "
	               << response.statusMsg << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = response.headers.begin(); it != response.headers.end(); ++it)
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << response.body;

	return responseString.str();
}

namespace http
{

	void TcpServer::setResponse()
	{
		log(m_serverMessage);
		std::cout << "Here" << std::endl;
		m_serverMessage = buildResponse(response);
		std::cout << m_serverMessage << std::endl;
	}
} // namespace http
