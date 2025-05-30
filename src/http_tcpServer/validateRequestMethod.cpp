#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>

bool http::TcpServer::validateGet() {
	if (request.path == "/")
		request.path = "login.html";
	std::string fullPath = "./content/" + request.path;
	std::ifstream htmlFile(fullPath.c_str());
	if (!htmlFile.is_open()) {
		std::cout << "FULL PATH: " << fullPath.c_str() << std::endl;
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
		std::cout << "\nSupposed to do something" << std::endl;
	else if (request.method == "DELETE")
		std::cout << "\nSupposed to do something" << std::endl;
	else {
		std::cout << "SET ERROR TRUE\n\n";
		setResponseError("405", "Method Not Allowed");
		return false;
	}
	return true;
}
