#pragma once

#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/HttpStructs.hpp"

class UploadManager
{
  private:
  public:
	static bool handleUpload(const Location &location, Client &client, const Server &serverInfo);
};

bool UploadManager::handleUpload(const Location &location, Client &client, const Server &serverInfo)
{
	std::string contentType;

	contentType = client.getRequest().headers.at("Content-Type");

	std::cout << contentType << std::endl;
}
