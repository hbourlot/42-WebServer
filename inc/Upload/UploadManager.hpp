#pragma once

#include "Client/Client.hpp"
#include "httpTcpServer/HttpStructs.hpp"

class UploadManager {
  private:
	static bool parseMultipart( const RouteContext &ctx, Client &Client, const ServerConfig &serverInfo );

  public:
	static bool handleUpload( const RouteContext &ctx, Client &client, const ServerConfig &serverInfo );
};
