#include "Upload/UploadManager.hpp"

bool UploadManager::handleUpload(const Location &matchLocation, Client &client, const ServerConfig &serverInfo) {

	std::map<std::string, std::string>::const_iterator it = client.getRequest()._headers.find("Content-Type");

	if (it == client.getRequest()._headers.end()) {
		client.getResponse().buildErrorResponse(HTTP_BAD_REQ, serverInfo);
		return false;
	}

	const std::string &contentType = it->second;

	if (contentType.find("multipart/form-data;") != std::string::npos) {
		if (UploadManager::parseMultipart(matchLocation, client, serverInfo))
			return (true);
	} else if (contentType.find("text/plain") != std::string::npos) {
	} else {
		client.getResponse().buildErrorResponse(HTTP_UNSUPPORTED_MEDIA, serverInfo);
	}
	return (false);
}
