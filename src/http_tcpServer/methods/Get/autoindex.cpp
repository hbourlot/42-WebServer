#include "http_tcpServer/Http_tcpServer_linux.hpp"

static std::string getParentPath(const std::string &path)
{
	std::string parent = path;

	if (!parent.empty() && parent[parent.length() - 1] == '/')
		parent.erase(parent.length() - 1);

	size_t prevfolder = parent.find_last_of('/');
	if (prevfolder != std::string::npos && prevfolder != 0)
		return (parent.substr(0, prevfolder));

	return ("/");
}

static bool hasIndexFile(const std::string &path, const Location &location)
{
	if (location.index.empty())
		return false;

	std::string indexPath = path + location.index;
	return (std::ifstream(indexPath.c_str()).is_open());
}

static std::string generateAutoIndexPage(const std::string &dirPath,
                                         const Location &location,
                                         httpRequest &request)
{
	std::string html;
	html += "<html>\n";
	html += "  <body>\n";
	html += "    <h1>Index of " + request.path + "</h1>\n";

	DIR *directory = opendir(dirPath.c_str());
	if (!directory)
		return "<html><body><h1>Unable to open "
		       "directory</h1></body></html>";

	struct dirent *entry;

	while ((entry = readdir(directory)) != NULL)
	{
		std::string d_name(entry->d_name);
		if (!d_name.compare("."))
			continue;

		std::string href;

		if (!d_name.compare(".."))
			href = getParentPath(request.path);
		else
			href = joinPath(request.path, d_name);

		html += "    <p><a href=\"" + href + "\">" + d_name + "</a></p>\n";
	}
	html += "</body>\n";
	html += "</html>";

	closedir(directory);
	return (html);
}

bool http::TcpServer::handleDirectoryListing(const std::string &filePath,
                                             const Location &location)
{
	if (hasIndexFile(filePath, location))
	{
		std::string indexPath = joinPath(filePath, location.index);
		setFileResponse("200", "OK", indexPath);
		return (true);
	}

	if (!location.autoIndex)
	{
		setFileResponse("404", "Not Found", infos.errorPage[404], true);
		return false;
	}

	std::string body = generateAutoIndexPage(filePath, location, request);

	setBodyResponse("200", "OK", body, "text/html");
	return true;
}