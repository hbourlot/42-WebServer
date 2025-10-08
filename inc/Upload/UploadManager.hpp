#pragma once

#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "httpTcpServer/HttpStructs.hpp"

class UploadManager
{
  private:
	static bool parseMultipart(const Location &location, Client &Client);

  public:
	static bool handleUpload(const Location &location, Client &client, const ServerConfig &serverInfo);
};
