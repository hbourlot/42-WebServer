#include <sstream>

#include "httpTcpServer/HttpTcpServerLinux.hpp"

std::string http::Request::getFileName() {
	std::string::size_type pos = _uri.rfind('/');
	if (pos == std::string::npos)
		return _uri;
	return _uri.substr(pos + 1);
}

bool isDirectory(const std::string &filePath) {
	struct stat s;
	if (stat(filePath.c_str(), &s) != 0)
		return (false);
	return (S_ISDIR(s.st_mode));
}

std::string getFilePath(const http::Request &req, const ServerConfig &server) {
	const Directory *dir = req.getFileDirectory();
	const Location *location = req.getMatchLocation();

	std::string root = server.root;
	std::string relativePath = req.getUri();
	if (location && !location->root.empty()) {
		root = location->root;
		if (!location->path.empty() && req.getUri().rfind(location->path, 0) == 0) {
			relativePath = req.getUri().substr(location->path.length());
		}
	}

	if (dir && !dir->path.empty()) {
		root = dir->root;
		relativePath = req.getUri().substr(dir->path.length());
	}

	if (relativePath.empty())
		relativePath = "/";

	return joinPath(root, relativePath);
}

std::string joinPath(const std::string &base, const std::string &sub) {
	if (base.empty())
		return sub;
	if (sub.empty())
		return base;

	bool baseHasSlash = (base[base.length() - 1] == '/');
	bool subHasSlash = (sub[0] == '/');
	if (baseHasSlash && subHasSlash) {

		return base + sub.substr(1);
	} else if (!baseHasSlash && !subHasSlash) {
		return base + "/" + sub;
	}

	return base + sub;
}

std::string ft_strtrim(const std::string &str) {
	unsigned int start = 0;
	unsigned int end = str.length();

	while (start < str.length() && std::isspace(str[start]))
		start++;
	while (end > start && std::isspace(str[end - 1]))
		end--;
	return (str.substr(start, end - start));
}

std::string dateString() {
	time_t timestamp;
	time(&timestamp);
	std::string date = ctime(&timestamp);
	if (!date.empty() && date[date.length() - 1] == '\n')
		date.erase(date.length() - 1);
	return (date);
}
time_t getActualTime(){
	time_t actual= time(0);
	return(actual);
}

// Returns true, if the state is the same. Otherwise we return false
bool containBrackets(std::string &line, bool &state, std::string extraStringToFind) {
	// If we find the new string
	if (!extraStringToFind.empty()) {
		return (line.find(extraStringToFind) != std::string::npos);
	}

	if (line.find('#') != std::string::npos) // Check if it is a comment
	{
		return true;
	}

	int openCount = 0;
	int closeCount = 0;
	for (size_t i = 0; i < line.size(); ++i) {
		if (line[i] == '{')
			openCount++;
		else if (line[i] == '}')
			closeCount++;
	}

	if (openCount > 1 || closeCount > 1       // If we have something like "server {{{{{"
	    || (openCount > 0 && closeCount > 0)) // If we have something like "server }{"
	{
		std::cerr << "More than one bracket in the same line" << std::endl;
		return false;
	}

	if (closeCount > 0) {
		if (state == true)
			state = false;

		else {
			std::cerr << "In configuration, is trying to close when its already closed" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	else if (openCount > 0) {
		if (state == false)
			state = true;

		else {
			std::cerr << "In configuration, is trying to open when its already open" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	return true;
}

// This method is to check especific cases like " {{{ location / }}}"
// This way we split the string by the location and see if we open him after
// Only for location cases
bool checkSplitString(const std::string &line, const std::string &sep, bool &isServerOpen) {
	std::string left = "";
	std::string right = "";
	std::string empty = "";

	std::size_t pos = line.find(sep);
	if (pos == std::string::npos)
		return true;

	left = line.substr(0, pos);
	right = line.substr(pos + sep.size());

	if (isServerOpen == true) // Since its already open, checks only the left
	{
		if (containBrackets(left, isServerOpen, empty) == false)
			return false;
		return true;
	}

	if (isServerOpen == false) // Checks the right side if it is open in the same line " location /upload {"
	{
		if (containBrackets(right, isServerOpen, empty) == false)
			return false;
	}

	return true;
}

void print(const char *src) {

	std::string a(src);
	std::cout << a << std::endl;
};

static std::string getExtension(const std::string &path) {
	size_t slashPos = path.find_last_of('/');
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
		return "";

	if (slashPos != std::string::npos && dotPos < slashPos)
		return "";

	return path.substr(dotPos + 1);
}

const Directory *getMatchDirectory(const std::string &path, const std::vector<Directory> &directories) {

	const Directory *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < directories.size(); ++i) {

		const std::string &locPath = directories[i].path;
		if (path.find(locPath) != std::string::npos && locPath.size() > matchLength) {
			matchedLocation = &directories[i];
			matchLength = locPath.size();
		}
	}
	return (matchedLocation);
}

const Location *getMatchLocation(const std::string &path, const std::vector<Location> &locations) {
	std::string ext = getExtension(path);
	if (!ext.empty()) {
		for (size_t i = 0; i < locations.size(); ++i) {
			if (locations[i].extension.find(ext) != std::string::npos) {
				return &locations[i];
			}
		}
	}
	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i) {
		const std::string &locPath = locations[i].path;
		if (path.find(locPath) != std::string::npos && locPath.size() > matchLength) {
			matchedLocation = &locations[i];
			matchLength = locPath.size();
		}
	}
	return (matchedLocation);
}

std::string createUploadBody() {
	std::string html;

	html += "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n<title>Error Occurred</title>\n";
	html += "<style>\n";
	html +=
	    "body { font-family: Arial, sans-serif; text-align: center; padding-top: 10%; background-color: #f9f9f9; }\n";
	html += ".error-box { display: inline-block; border-radius: 10px; padding: 2em 3em; background: #fff; box-shadow: "
	        "0 2px 8px rgba(0,0,0,0.1); }\n";
	html += "h1 { margin-bottom: 0.5em; color: #5ad94f; }\n";
	html += "p { color: #555; margin-bottom: 1.5em; }\n";
	html += "a.button { text-decoration: none; color: white; background: #007bff; padding: 0.7em 1.5em; border-radius: "
	        "5px; font-weight: bold; }\n";
	html += "a.button:hover { background: #0056b3; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<div class=\"error-box\">";
	html += "<h1>Upload OK</h1>";
	html += "<a href=\"/\" class=\"button\">Home</a>\n";
	html += "</div>\n";
	html += "</body>\n</html>\n";

	return html;
}

ssize_t writeAll(int fd, const char *buf, size_t len) {
	size_t total = 0;

	while (total < len) {
		ssize_t ret = write(fd, buf + total, len - total);

		if (ret < 0) {
			if (errno == EINTR)
				continue; // retry
			Logs::log(LOGS_ERROR, "Write on fd failed");
			return -1; // error
		}
		total += ret;
	}
	return total;
}


void removeComment(std::string &line)
{
	size_t position = line.find('#');
	if (position != std::string::npos)
		line = line.substr(0, position);
	return;
}

bool isDigits(std::string str)
{
    for (size_t i = 0; str.size() > i; i++)
	{
		if (!(str[i] >= 48 && str[i] <= 57 ))
			return false;
	}
	return true;
}
