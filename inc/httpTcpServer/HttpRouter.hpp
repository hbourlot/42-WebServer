#pragma once
#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "HttpUtils.hpp"

enum VALIDATION_STATUS {

	VALID_OK = 1,
	VALID_METHOD_NOT_ALLOWED,
	VALID_NOT_FOUND,
	VALID_FORBIDDEN,
	VALID_REDIRECT_REQUIRED,
	VALID_ERROR,

};

class HttpRouter {
  public:
	static VALIDATION_STATUS validateRequest( Client &client, const ServerConfig &server );
	static void handleMethods( Client &client, const ServerConfig &server );

  private:
	static void handleGet( Client &client, const ServerConfig &server, const Location &location );
	static void handlePost( Client &client, const ServerConfig &server, const Location &location );
	static void handleDelete( Client &client, const ServerConfig &server, const Location &location );

	// Can add also other http methods here
};

const Location *getMatchLocation( const std::string &path, const std::vector< Location > &locations );