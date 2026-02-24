#include "httpTcpServer/HttpTcpServerLinux.hpp"

static std::string getParentPath(const std::string &path) {
	std::string parent = path;

	if (!parent.empty() && parent[parent.length() - 1] == '/')
		parent.erase(parent.length() - 1);

	size_t prevfolder = parent.find_last_of('/');
	if (prevfolder != std::string::npos && prevfolder != 0)
		return (parent.substr(0, prevfolder));

	return ("/");
}

static bool hasIndexFile(const std::string &path, const Location &matchLocation) {

	if (matchLocation.index.empty())
		return false;

	std::string indexPath = joinPath(path, matchLocation.index);
	return (std::ifstream(indexPath.c_str()).is_open());
}

static std::string generateAutoIndexPage(const std::string &dirPath, http::Request &request) {
	std::string html;
	html += "<html>\n";
	html += "  <body>\n";
	html += "    <h1>Index of " + request.getUri() + "</h1>\n";

	DIR *directory = opendir(dirPath.c_str());
	if (!directory)
		return "<html><body><h1>Unable to open "
		       "directory</h1></body></html>";

	struct dirent *entry;

	while ((entry = readdir(directory)) != NULL) {
		std::string d_name(entry->d_name);
		if (!d_name.compare("."))
			continue;

		std::string href;

		if (!d_name.compare(".."))
			href = getParentPath(request.getUri());
		else
			href = joinPath(request.getUri(), d_name);

		html += "    <p><a href=\"" + href + "\">" + d_name + "</a></p>\n";
	}
	html += "</body>\n";
	html += "</html>";

	closedir(directory);
	return (html);
}

void http::Router::handleDirectoryListing() {
	http::Request &request = _client.getRequest();
	http::Response &response = _client.getResponse();

	if (hasIndexFile(_request.getFullPath(), *_request.getMatchLocation())) {
		std::string indexPath = joinPath(_request.getFullPath(), _request.getMatchLocation()->index);
		response.buildFileResponse(HTTP_OK, indexPath, _serverConfig);
		return;
	}

	if (!_request.getMatchLocation()->autoIndex) {
		response.buildErrorResponse(HTTP_NOT_FOUND, _serverConfig);
		return;
	}

	std::string body = generateAutoIndexPage(_request.getFullPath(), request);

	response.buildResponse(HTTP_OK, body);

	return;
}