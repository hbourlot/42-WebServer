#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <iostream>

bool http::TcpServer::parseCgi(const Location loc, std::string &filePath, Client *client) {

	// if (loc.methods.empty() || m_scriptName.empty()) {
	// 	return NULL;
	// }

	ICgi *cgi = new PythonCgi(client, filePath);
	if (!cgi) {
		std::cerr << "Error: Error allocating memory of Cgi." << std::endl;
	} else {
		client->addCgi(cgi);
		client->executeCgi();
	}
	return true;
}
