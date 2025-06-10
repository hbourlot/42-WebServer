#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{
	// void TcpServer::setResponseError(const std::string &statusCode,
	//                                  const std::string &statusMsg)
	// {
	// 	response.setResponseError(statusCode, statusMsg);
	// 	setResponse(); // <- opcional: si querés que se construya el mensaje
	// 	               // completo de inmediato
	// }

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

	static std::string generateAutoIndexPage(std::string &dirPath,
	                                         const Location *location,
	                                         httpRequest &request)
	{
		std::string autoindex =
		    "<html>\n<body>\n<h1>Index of " + request.path + "</h1>\n";
		DIR *directory;

		directory = opendir(dirPath.c_str());
		if (!directory)
			return "<html><body><h1>Unable to open "
			       "directory</h1></body></html>";

		struct dirent *dirent;
		dirent = readdir(directory);

		while (dirent != NULL)
		{
			std::string d_name(dirent->d_name);
			if (d_name.compare("."))
			{

				if (d_name.compare(".."))

					autoindex +=
					    "<p><a href=" + joinPath(request.path, d_name) + ">" +
					    d_name + "</a></p>\n";
				else
				{

					std::string parentPath = getParentPath(request.path);
					autoindex += "<p><a href=" + parentPath + ">" + d_name +
					             "</a></p>\n";
				}
			}
			dirent = readdir(directory);
		}
		autoindex = autoindex + "</body>\n</html>";
		closedir(directory);
		return (autoindex);
	}

	bool TcpServer::handleGetRequest(const Location *location)
	{
		// if (!location->cgi_path.empty())
		// {
		// 	std::cout << "HERE CGI GET" << std::endl;
		// 	return true;
		// }

		std::string filePath = getFilePath(request.path, location);

		if (isDirectory(filePath))
		{
			if (!location->index.empty())
			{
				std::string indexPath = filePath + location->index;
				if (std::ifstream(indexPath.c_str()).is_open())
				{
					setFileResponse("200", "OK", indexPath);
					return true;
				}
			}

			if (!location->autoIndex)
			{
				setFileResponse("404", "Not Found", infos.errorPage[404], true);
				return false;
			}

			std::string body =
			    generateAutoIndexPage(filePath, location, request);
			response.statusCode = "200";
			response.statusMsg = "OK";
			response.body = body;
			response.addHeader("Content-Type", "text/html");
			return true;
		}

		if (!std::ifstream(filePath.c_str()).is_open())
		{
			setFileResponse("404", "Not Found", infos.errorPage[404], true);
			return false;
		}

		setFileResponse("200", "OK", filePath);
		return true;
	}

} // namespace http
