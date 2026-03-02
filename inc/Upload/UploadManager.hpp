#pragma once

#include "Client/Client.hpp"

class UploadManager {
  private:
	static bool parseMultipart( const Location &matchLocation, Client &Client, const ServerConfig &serverConfig );

  public:
	static bool handleUpload( const Location &matchLocation, Client &client, const ServerConfig &serverConfig );
};
