#pragma once
#include "Client/Client.hpp"
#include "Client/ClientEventProcessor.hpp"
#include "Config/Configs.hpp"
#include "HttpUtils.hpp"

enum VALIDATION_STATUS {

	VALID_OK = 1,
	VALID_METHOD_NOT_ALLOWED,
	VALID_NOT_FOUND,
	VALID_FORBIDDEN,
	VALID_REDIRECT_REQUIRED,
	VALID_IS_CGI,

};

namespace http {

	class ClientEventProcessor;
	class Router {
	  public:
		static VALIDATION_STATUS validateRequest( Client &client, const ServerConfig &server );
		static void routeRequest( Client &client, const ServerConfig &server, ClientEventProcessor &processor );

	  private:
		static bool isCgiRequest( const httpRequest &request, const Location &location );
		static void handleCgiRequest( Client &client, const ServerConfig &server, const Location &location,
		                              ClientEventProcessor &processor );
		static void handleStaticRequest( Client &client, const ServerConfig &server, const Location &location );

		static void handleGet( Client &client, const ServerConfig &server, const Location &location );
		static void handlePost( Client &client, const ServerConfig &server, const Location &location );
		static void handleDelete( Client &client, const ServerConfig &server, const Location &location );
		static void handleDirectoryListing( Client &client, const ServerConfig &server, const std::string &filePath,
		                             const Location &location );

		// Can add also other http methods here
	};

} // namespace http

const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations );