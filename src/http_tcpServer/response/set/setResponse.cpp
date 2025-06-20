#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

static std::string buildResponse(const httpResponse &response,
                                 const httpRequest &request) {
	std::ostringstream responseString;
	responseString << request.serverProtocol + " " << response.statusCode << " "
	               << response.statusMsg << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = response.headers.begin(); it != response.headers.end(); ++it)
		responseString << it->first << ": " << it->second << "\r\n";

	responseString << "\r\n";
	responseString << response.body;

	return responseString.str();
}

namespace http {

	void TcpServer::setResponse() {
		log(_serverMessage);
		_serverMessage = buildResponse(_response, _request);
		std::cout << _serverMessage << std::endl;
	}
} // namespace http
