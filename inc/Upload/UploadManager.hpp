#pragma once

#include "Client/Client.hpp"
#include "httpTcpServer/HttpStructs.hpp"

class UploadManager {
  private:
	static bool parseMultipart( const Location &location, Client &Client, const ServerConfig &serverInfo );

  public:
	static bool handleUpload( const Location &location, Client &client, const ServerConfig &serverInfo );
};
