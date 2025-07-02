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

	static void handlePost(...);
	static void handleDelete(...);
	// Otros métodos de lógica HTTP
};
