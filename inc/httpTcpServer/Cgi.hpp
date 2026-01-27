#pragma once
#include "Config/Configs.hpp"
#include "Client/Client.hpp"
#include "HttpStructs.hpp"
#include <cstring>
#include <fcntl.h>
#include <ios>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class Client;

namespace http {
	const std::string CGI_NO_OUTPUT_PAGE =
	    "<!DOCTYPE html>\n"
	    "<html><head><title>CGI Error</title><style>\n"
	    "body{font-family:Arial,sans-serif;background:#f4f4f4;display:flex;justify-content:center;align-items:"
	    "center;"
	    "height:100vh;margin:0}\n"
	    ".container{background:white;padding:40px;border-radius:8px;box-shadow:0 2px 10px "
	    "rgba(0,0,0,0.1);text-align:center;max-width:500px}\n"
	    "h1{color:#e74c3c;margin-bottom:20px}p{color:#555;line-height:1.6}.error-code{font-size:72px;color:#e74c3c;"
	    "font-weight:bold;margin:20px 0}\n"
	    "</style></head><body>\n"
	    "<div class='container'><div class='error-code'>⚠️</div>\n"
	    "<h1>CGI Script Error</h1><p><strong>No Output Received</strong></p>\n"
	    "<p>The CGI script executed but did not produce any output.</p>\n"
	    "<p>This could indicate an issue with the script or missing output headers.</p>\n"
	    "</div></body></html>";
	class Cgi {

	  public:
		Cgi( const http::Request &request, const std::string &scriptPath, const ServerConfig &serverInfo,
		     Client *client );

		~Cgi();

		void executeCgi( void );
		Response getResponse() const;
		http::Request getRequest() const;
		std::string getFilePath() const;
		std::string getBody() const;
		int &getStatus();
		int getStatus() const;
		std::vector< std::string > getArgv() const;
		int getPollFd() const;
		pid_t getPid() const;
		const int *getInputPipe() const;
		const int *getOutputPipe() const;
		void killProcess();
		Client *getClient() const;
		bool hasDataToRead();
		void writeToCgi();
		size_t getBodyBytesWritten() const;

		std::string &getOutputBuffer() {
			return _outputBuffer;
		};

	  private:
		int _status;
		SocketFD _clientFD;
		http::Request _request;
		Response _response;
		ServerConfig _serverInfo;
		std::string _filePath;
		const sockaddr_in _clientAddress;
		int _bytesReceived;
		size_t _bodyBytesWritten;
		std::string _body;
		Client *_client; // Back-reference to client
		std::string _bodyFileName;

		std::vector< char * > _envp;
		std::vector< char * > _argv;
		std::vector< std::string > _envStrings;

		// Accumulate CGI stdout across poll cycles
		std::string _outputBuffer;

		// Pipe handling
		int _pipefd[ 2 ];
		int _inputPipe[ 2 ];
		int _outputPipe[ 2 ];
		pid_t _pid;

		void buildEnvStrings();
		void doDupOneWay();
		void doDupTwoWay();
		void closeForOneWay();
		void closeForTwoWay();
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
// All HTTP headers from the request, upperCased, dashes replaced by
// underscores, and prefixed with HTTP_ (e.g., HTTP_USER_AGENT, HTTP_COOKIE,
// etc.)

// Optional/Advanced (but good to have):
// AUTH_TYPE (if using authentication)
// REMOTE_USER (if using authentication)
// HTTP_REFERER
// HTTP_USER_AGENT
// HTTP_COOKIE
// Any other HTTP headers as HTTP_*