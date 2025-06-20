#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

static void parseRequestQueries(httpRequest &request) {
	std::string fullPath = request.path;
	std::string::size_type qpos = fullPath.find('?');

	if (qpos != std::string::npos) {
		request.queryString = fullPath.substr(qpos + 1);
		request.path = fullPath.substr(0, qpos);
	} else {
		request.queryString = "";
	}
}

static std::string getPathInfo(std::string &requestPath,
                               std::string scriptExt) {

	std::string pathInfo;
	std::string::size_type extPos = requestPath.find(scriptExt);
	if (extPos != std::string::npos) {
		extPos += scriptExt.length();

		if (extPos < requestPath.length()) {
			pathInfo = requestPath.substr(extPos);
		}
	}

	return pathInfo.empty() ? "" : pathInfo;
}

static std::string getPathTranslated(const std::string &root,
                                     const std::string pathInfo) {

	std::string result = root;
	if (!result.empty() && result.back() == '/' && !pathInfo.empty() &&
	    pathInfo.front() == '/')
		result.pop_back();
	return result + pathInfo;
}

static void parsePath(httpRequest &request, const Server &serverInfo) {
	std::string &requestPath = request.path;
	std::string pathInfo;

	for (size_t i = 0; i < serverInfo.locations.size(); ++i) {
		if (!serverInfo.locations[i].cgi_extension.empty()) {
			std::vector<std::string> scriptExt =
			    serverInfo.locations[i].cgi_extension;

			for (size_t j = 0; j < scriptExt.size(); ++j) {
				std::string &value = scriptExt[j];
				std::string::size_type extPos = requestPath.find(value);

				if (requestPath.find(value) != std::string::npos &&
				    requestPath.find(value) + value.length() <=
				        requestPath.length()) {

					char nextChar = requestPath[extPos + value.length()];
					if (nextChar != '/')
						continue;

					request.pathInfo = getPathInfo(requestPath, value);
					if (!request.pathInfo.empty()) {
						requestPath.erase(requestPath.length() -
						                  request.pathInfo.length());
						request.pathTranslated = getPathTranslated(
						    serverInfo.locations[i].root, request.pathInfo);
					}
					return;
				}
			}
		}
	}
	if (request.pathInfo.empty()) {
		request.pathInfo = "";
		request.pathTranslated = "";
	}
}

static void parseRequestHeaders(httpRequest &request,
                                std::istringstream &request_stream,
                                std::string &line) {
	size_t idx;

	while (std::getline(request_stream, line)) {
		if (line == "\r" || line == "")
			break;
		idx = line.find(":");
		if (idx != std::string::npos) {

			std::string key = line.substr(0, idx);
			std::string value = line.substr(idx + 1);
			key = ft_strtrim(key);
			value = ft_strtrim(value);
			request.headers[key] = value;
		}
	}
}

void parseRequest(httpRequest &request, const std::string &requestContent,
                  Server &serverInfo) {
	std::istringstream request_stream(requestContent);
	std::string line;
	std::getline(request_stream, line);
	std::istringstream first_line(line);

	first_line >> request.method >> request.path >> request.serverProtocol;
	parseRequestQueries(request);
	parsePath(request, serverInfo);

	// // ✅ Print debug info
	// std::cout << "Parsed Request:" << std::endl;
	// std::cout << "  Method: " << request.method << std::endl;
	// std::cout << "  Path: " << request.path << std::endl;
	// std::cout << "  Path Info: " << request.pathInfo << std::endl;
	// std::cout << "  Path Translated: " << request.pathTranslated <<
	// std::endl; std::cout << "  Query String: " << request.queryString <<
	// std::endl; exit(1);
	parseRequestHeaders(request, request_stream, line);

	std::string body;
	while (std::getline(request_stream, line))
		body += line + "\n";
	request.body = body;
}
