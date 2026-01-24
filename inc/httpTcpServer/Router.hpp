#pragma once
#include "Client/Client.hpp"
#include "Client/ClientEventProcessor.hpp"
#include "Config/Configs.hpp"

namespace http {

	class ClientEventProcessor;
	class Router {
	  public:
		static VALIDATION_STATUS validateRequest( Client &client, const ServerConfig &server );
		static bool routeCgiRequest( Client &client, const ServerConfig &server, const RouteContext &ctx,
		                             ClientEventProcessor &processor );
		static void routeStaticRequest( Client &client, const ServerConfig &server, const RouteContext &ctx );

	  private:
		static void launchCgi( Client &client, const ServerConfig &server, const RouteContext &ctx,
		                       ClientEventProcessor &processor );
		static void handleGet( Client &client, const ServerConfig &server, const RouteContext &ctx );
		static void handlePost( Client &client, const ServerConfig &server, const RouteContext &ctx );
		static void handleDelete( Client &client, const ServerConfig &server, const RouteContext &ctx );
		static void handleDirectoryListing( Client &client, const ServerConfig &server, const std::string &filePath,
		                                    const RouteContext &ctx );

		// Can add also other http methods here
	};

} // namespace http

// const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations );