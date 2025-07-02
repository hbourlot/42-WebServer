#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <netinet/in.h>
#include <string>
// #include "http_tcpServer/Http_tcpServer_Linux.hpp"

// --- START DEBUG
// Location:
//   path: /cgi-bin
//   methods: GET, POST
//   root: ./var/www/cgi-bin
//   index:
//   redirection:
//   cgi_extension: .py
//   cgi_path: /opt/homebrew/bin/python3
//   cgi map: [.py]=/opt/homebrew/bin/python3
//   uploadEnable: false
//   uploadStore:
//   autoIndex: false
// --- END DEBUG

// struct Location
// {
// 	std::string path;                 // location --> /upload <--
// 	std::vector<std::string> methods; // method POST GET DELETE
// 	std::string root;
// 	std::string index;
// 	std::string redirection; // http://example.com;
// 	std::vector<std::string> cgi_extension;
// 	std::vector<std::string> cgi_path;
// 	std::map<std::string, std::string> cgi;
// 	bool uploadEnable;
// 	std::string uploadStore;
// 	bool autoIndex;
// };

bool http::TcpServer::parseCgi(const Location loc, std::string &filePath,
                               sockaddr_in &clientAddress,
                               httpRequest &request) {

	// if (loc.methods.empty() || m_scriptName.empty()) {
	// 	return false;
	// }

	Cgi cgi(request, filePath, clientAddress, _serverInfo);
	// Cgi cgi(_request, filePath, clientAddress, _serverInfo);

	this->_cgi.push_back(cgi);
	return true;
};
