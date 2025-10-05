#include "Upload/UploadManager.hpp"

bool UploadManager::handleUpload(const Location &location, Client &client, const Server &serverInfo)
{
	std::string contentType;

	contentType = client.getRequest().headers.at("Content-Type");

	std::cout << contentType << std::endl;

	if (contentType.find("multipart/form-data;") != std::string::npos)
	{
		UploadManager::parseMultipart(location, client);
	}
	return (false);
}
