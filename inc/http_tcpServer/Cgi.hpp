#pragma once
#include "HttpStructs.hpp"
#include <map>
#include <netinet/in.h>
#include <string>
#include <vector>

enum Info {
	infoMethod = 1,
	infoQueryString,
	infoContentType,
	infoFileName,
	infoScriptName,
	infoServerName,
	infoServerPort,
	infoServerProtocol,
	infoServerSoftware,
	infoGetWayInterface,
	infoRemotePort,
	infoPathInfo,
	infoPathTranslated,
	infoHttpUserAgent,
	infoAcceptLanguage,
	infoCookie,
	infoReferer,
	infoHeader
};

namespace http {
	class Cgi {

	  public:
		Cgi(httpRequest &request, std::string &filePath,
		    sockaddr_in &clientAddress);
		~Cgi();

		// // CGI
		static const std::set<std::string> validCgiExtensions;
		static bool isValidCgiExtension(const std::string &ext);
		static std::set<std::string>
		createValidCgiExtensions() // ! maybe must be outside
		{
			std::set<std::string> s;
			s.insert(".py");
			s.insert(".cgi");
			return s;
		}

	  private:
		httpRequest _request;
		const char *_filePath;
		sockaddr_in _clientAddress;

		std::map<std::string, std::string> _cgiVars;
		std::vector<char *> _envp;
		std::vector<char *> _argv;

		// Response
		httpResponse _response;

		// Pipe handling
		int _inputPipe[2];
		int _outputPipe[2];
		pid_t _pid;

		void buildCgiVars();
		std::string
		getServerName(std::map<std::string, std::string> &headers) const;
		std::string
		getServerPort(std::map<std::string, std::string> &headers) const;
	};

}; // namespace http

// Essential CGI Environment Variables
// REQUEST_METHOD
// The HTTP method (GET, POST, etc.)

// QUERY_STRING
// The raw query string (everything after ? in the URL)

// CONTENT_TYPE
// The MIME type of the request body (for POST/PUT)

// CONTENT_LENGTH
// The length of the request body (for POST/PUT)

// SCRIPT_NAME
// The path to the CGI script (relative to the web root)

// SCRIPT_FILENAME
// The absolute path to the CGI script

// SERVER_NAME
// The server’s hostname or IP (from the Host header or config)

// SERVER_PORT
// The port your server is listening on (as a string)

// SERVER_PROTOCOL
// The HTTP version (e.g., HTTP/1.1)

// SERVER_SOFTWARE
// Your server’s name/version (e.g., Webserv/1.0)

// GATEWAY_INTERFACE
// The CGI version (usually CGI/1.1)

// REMOTE_ADDR
// The client’s IP address

// REMOTE_PORT
// The client’s port

// PATH_INFO
// Extra path info after the script name (if any)

// PATH_TRANSLATED
// The translated path of PATH_INFO (if any)

// HTTP_ variables*
// All HTTP headers from the request, uppercased, dashes replaced by
// underscores, and prefixed with HTTP_ (e.g., HTTP_USER_AGENT, HTTP_COOKIE,
// etc.)

// Optional/Advanced (but good to have):
// AUTH_TYPE (if using authentication)
// REMOTE_USER (if using authentication)
// HTTP_REFERER
// HTTP_USER_AGENT
// HTTP_COOKIE
// Any other HTTP headers as HTTP_*