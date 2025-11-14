#pragma once
#include "Config/Configs.hpp"
#include "HttpStatus.hpp"
#include "HttpStructs.hpp"

class ResponseBuilder {
  public:
	static httpResponse buildResponse( const HttpStatusCode &status, const std::string &body,
	                                   const std::string &headerKey = "", const std::string &headerValue = "",
	                                   httpRequest *req = NULL );

	static httpResponse buildErrorResponse( const HttpStatusCode &status );
	static httpResponse buildRedirect( const HttpStatusCode &status, const std::string &url );
	static httpResponse buildFileResponse( const HttpStatusCode &status, const std::string &filePath,
	                                       const ServerConfig &server, bool isError = false );

  private:
	static std::string readFileContent( const std::string &filePath );
	static std::string getContentType( const std::string &filePath );
};
