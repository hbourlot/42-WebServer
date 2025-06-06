#include "http_tcpServer/Http_tcpServer_linux.hpp"
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

// if (CgiHandler::isCgiRequest(request)) {
// 	int i = 0;
// 	std::string ext =
// 		getLocationFieldAsString(infos.locations, "cgi_extension");
// 	std::vector<std::string> ext_splitted = split(ext, ' ');

// 	// Maybe verify all cgi_extensions instead of just once by once
// 	while (CgiHandler::isValidCgiExtension(ext_splitted[i]))
// 		i++;
// 	if (i == ext.size()){
// 		// handleCgiRequest
// 	}
// }

bool http::TcpServer::parseCgi(const Location loc) {

	// if (loc.methods.empty() || m_scriptName.empty())
	// {
	// return false;
	// }

	struct Cgi foo;

	foo.method = request.method;
	foo.queryString = request.rawQueries;
	foo.requestBody = request.body;
	foo.contentLength = request.body.length();
	foo.contentType = request.headers["Content-Type"];

	this->m_cgi.push_back(foo);
	return true;
};
