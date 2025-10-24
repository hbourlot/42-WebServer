#include "httpTcpServer/HttpTcpServerLinux.hpp"

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

	// std::string indexPath = path + location.index;
	std::string indexPath = joinPath(path, location.index);
	// std::cout << "\033[0;31m" << indexPath << "\033[0m" << std::endl;
	return (std::ifstream(indexPath.c_str()).is_open());
}

static std::string generateAutoIndexPage(const std::string &dirPath, const Location &location, httpRequest &request)
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

void handleDirectoryListing(Client &client, const ServerConfig &server, const std::string &filePath,
                            const Location &location)
{
	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	if (hasIndexFile(filePath, location))
	{
		std::string indexPath = joinPath(filePath, location.index);
		response = ResponseBuilder::buildFileResponse(HTTP_OK, indexPath, server);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));

		// return (true);
	}

	if (!location.autoIndex)
	{
		response = ResponseBuilder::buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
		client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));
		// return false;
	}

	std::string body = generateAutoIndexPage(filePath, location, request);

	response = ResponseBuilder::buildResponse(HTTP_OK, body, "text/html");
	client.appendToWriteBuffer(ResponseBuilder::buildResponseString(response, request));

	// return true;
}