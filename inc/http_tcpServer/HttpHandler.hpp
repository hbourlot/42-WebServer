#pragma once
#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "HttpUtils.hpp"

class HttpHandler
{
  public:
	static void handle(Client &client, const Server &server);

  private:
	static void handleGet(Client &client, const Server &server, const Location &location);
	static void handlePost(Client &client, const Server &server, const Location &location);
	static void handleDelete(Client &client, const Server &server, const Location &location);

	// Can add also other http methods here
};
