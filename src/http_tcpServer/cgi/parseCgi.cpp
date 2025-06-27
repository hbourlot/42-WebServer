#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstdio>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <vector>

bool http::TcpServer::parseCgi(const Location loc, sockaddr_in &clientAddress, httpRequest &request,
							   pollfd &clientSocket, std::vector<pollfd> &fds) {

	for (ssize_t i = 0; i < _cgi.size(); ++i) {

		if (_cgi[i]->getStatus() != Cgi::RUNNING) {
			_cgi[i]->executeCgi(fds);
			_cgi[i]->markAsRunning();
		}
	}
	// if (loc.methods.empty() || m_scriptName.empty()) {
	// 	return false;
	// }

	std::string filePath = getFilePath(request.path, loc);
	std::string prototypeFilePath = filePath.substr(1);

	Cgi cgi(_request, prototypeFilePath, clientAddress, _serverInfo, clientSocket);
	cgi.executeCgi(fds);

	_test = &cgi;
	_test->markAsRunning();
	_test->readCgiOutput();
	// exit(0);
	// this->_cgi.push_back(&cgi);
	return true;
};
