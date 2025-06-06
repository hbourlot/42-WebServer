#include "CGI/CgiHandler.hpp"
#include "Config/Configs.hpp"
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

// bool http::CgiHandler::parseCgi(httpRequest request)
bool http::CgiHandler::parseCgi(Location loc, httpRequest &request)
{

	// if (loc.methods.empty() || m_scriptName.empty())
	// {
	// return false;
	// }

	// m_method = "GET"; // !! ???	loc.methods[0];
	// m_queryString = request.queries;
	// m_requestBody = request.body;
	// m_contentLength = request.body.length();
	// m_contentType = request.headers["Content-Type"];

	return true;
};
