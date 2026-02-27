#pragma once
#include "Client/Client.hpp"
#include "Client/ClientEventProcessor.hpp"
#include "Config/Configs.hpp"
#include "httpTcpServer/HttpStatus.hpp"
namespace http {

	class ClientEventProcessor;
	class Cgi;

	class Router {

	  private:
		Client& _client;
		Request& _request;
		Response& _response;
		ServerConfig& _serverConfig;
		ClientEventProcessor& _eventProcessor;
		std::set<std::string> _protectedRoutes;

		bool isProtectedRoute(const std::string& uri);
		bool checkRedirects();
		bool checkAllowedMethods();
		void resolvePath();
		void executeRequest();

	  public:
		Router(Client& client, ClientEventProcessor& processor);

		void process();


		private:

		bool handleRouteProtected();
		void launchCgi();
		void handleGet();
		void handlePost();
		void handleDelete();
		void handleDirectoryListing();

		// Can add also other http methods here
	};

} // namespace http

// const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations );