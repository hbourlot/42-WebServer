#include "Upload/UploadManager.hpp"

bool UploadManager::handleUpload(const Location &location, Client &client, const ServerConfig &serverInfo)
{
	std::string contentType;

	httpRequest &request = client.getRequest();
	httpResponse &response = client.getResponse();

	contentType = client.getRequest().headers.at("Content-Type");

	std::cout << contentType << std::endl;

	if (contentType.find("multipart/form-data;") != std::string::npos)
	{
		if (UploadManager::parseMultipart(location, client))
			return (true);
	}
	else if (contentType.find("text/plain") != std::string::npos)
	{
	}
	else
	{
		response = ResponseBuilder::buildErrorResponse(HTTP_UNSUPPORTED_MEDIA);
	}
	return (false);
}
