#pragma once
#include "Client/Client.hpp"
#include "Config/Configs.hpp"
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
	class EventProcessor;
	class Cgi {

	  public:
		Cgi(const http::Request& request, const ServerConfig& serverInfo, Client* client);

		~Cgi();

		bool executeCgi(void);
		void killProcess();
		std::string getFilePath() const;
		pid_t getPid() const;
		int getStatus() const;
		int& getStatus();
		const int* getOutputPipe() const;
		int getOutputPipeFd() const;
		Client* getClient() const;
		IN_OUT_STATE& getState();
		std::string& getReadBuffer();
		void dumpEnvStrings() const;
		void dumpEnvp() const;

		bool hasFinished();
		bool hasSuccessfullyFinished() const;

	  private:
		http::Request _request;
		ServerConfig _serverInfo;
		Client* _client; // Back-reference to client
		pid_t _pid;
		int _status;
		int _stdinFd;
		const sockaddr_in _clientAddress;
		int _outputPipe[2];
		std::string _filePath;
		std::vector<char*> _envp;
		std::vector<std::string> _envStrings;
		IN_OUT_STATE _state;
		bool _hasFinished;
		std::string _outputBuffer;
		std::string _bodyFileName;

		void dupCgiFds();
		void buildEnvStrings();
		int prepareCgiInputFile();
		int validateFilePermision();
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