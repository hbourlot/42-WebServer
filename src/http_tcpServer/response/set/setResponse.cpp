#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

// static std::string buildResponse(const httpResponse &response,
//                                  const httpRequest &request) {
// 	std::ostringstream responseString;
// 	responseString << request.serverProtocol + " " << response.statusCode << " "
// 	               << response.statusMsg << "\r\n";

// 	std::map<std::string, std::string>::const_iterator it;
// 	for (it = response.headers.begin(); it != response.headers.end(); ++it)
// 		responseString << it->first << ": " << it->second << "\r\n";

// 	responseString << "\r\n";
// 	responseString << response.body;

// 	return responseString.str();
// }

// namespace http {
// 	void TcpServer::prepareResponse(const HttpStatusCode &status,
// 	                                const std::string &body,
// 	                                const std::string &headerKey,
// 	                                const std::string &headerValue) {
// 		_response.statusCode = status.code;
// 		_response.statusMsg = status.message;
// 		_response.body = body;

// 		if (!headerKey.empty() && !headerValue.empty())
// 			_response.addToHeader(headerKey, headerValue);

// 		_response.setDefaultHeaders(_request);
// 		setResponse();
// 	}

// 	void TcpServer::setResponse() {
// 		_serverMessage = buildResponse(_response, _request);
// 	}
// } // namespace http
