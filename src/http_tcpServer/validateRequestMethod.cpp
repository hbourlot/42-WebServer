#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>

bool http::TcpServer::validateGet() {
	if (request.path == "/")
		request.path = "login.html";
	logDebugger(request.path);
	std::string fullPath = "./content/" + request.path;
	std::ifstream htmlFile(fullPath.c_str());
	if (!htmlFile.is_open()) {
		setResponseError("404", "Not Found");
		return false; // TODO: throw error would be better
	}
	setHtmlResponse(htmlFile);
	return (true);
}

bool http::TcpServer::validateRequestMethod() {
	if (request.method == "GET") {
		if (validateGet() == false) {
			return (false);
		}
	} else if (request.method == "POST")
		std::cout << "Supposed to do something" << std::endl;
	else if (request.method == "DELETE")
		std::cout << "Supposed to do something" << std::endl;
	else {
		setResponseError("405", "Method Not Allowed");
		return false;
	}
	return true;
}
