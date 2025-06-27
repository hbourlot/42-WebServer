#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>

bool http::TcpServer::parseCgi(const Location loc, std::string &filePath,
                               sockaddr_in &clientAddress, httpRequest &request,
                               pollfd &clientSocket) {

	// if (loc.methods.empty() || m_scriptName.empty()) {
	// 	return false;
	// }

	Cgi cgi(_request, filePath, clientAddress, _serverInfo, clientSocket);
	// Cgi cgi(_request, filePath, clientAddress, _serverInfo);

	this->_cgi.push_back(cgi);
	return true;
};
