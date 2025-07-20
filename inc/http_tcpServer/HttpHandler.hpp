#pragma once
#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "HttpStructs.hpp"
#include "HttpUtils.hpp"
#include "Http_tcpServer_linux.hpp"

namespace http {

	struct ClientManager;

	class HttpHandler {
	  public:
		static void handle(ClientManager *object, Client &client, const Server &server);

	  private:
		static bool handleCgi(ClientManager *clientManager, Client *client, const httpRequest &request,
		                      const Location &location);
		static void handleGet(Client &client, const Server &server, const Location &location);
		static void handlePost(Client &client, const Server &server, const Location &location);

		static void handleDelete(Client &client, const Server &server, const Location &location);
		// Can add also other http methods here
	};

} // namespace http
