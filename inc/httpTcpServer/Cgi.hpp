#pragma once
#include "Config/Configs.hpp"
#include "HttpStructs.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <vector>

namespace http {
	class Cgi {

	  public:
		Cgi( const httpRequest &request, std::string &filePath, const sockaddr_in &clientAddress,
		     const ServerConfig &serverInfo );

		~Cgi();

		enum CgiStatus { CGI_NOT_STARTED, CGI_RUNNING, CGI_FINISHED, CGI_TOO_LARGE, CGI_ERROR = -1 };

		void executeCgi( std::vector< pollfd > &fds );
		HttpResponse getResponse() const;
		httpRequest getRequest() const;
		std::string getFilePath() const;
		std::string getBody() const;
		CgiStatus getStatus() const;
		std::vector< std::string > getArgv() const;
		int getPollFd() const;
		void registerPollFd( std::vector< pollfd > &fds ) const;
		void markAsRunning();
		bool readCgiOutput( void ( *updateStatusPtr )() = NULL);
		bool isCgiFinished();
		bool hasDataToRead();
		bool processCgiOut();

	  private:
		CgiStatus _status;
		SocketFD _clientFD;
		httpRequest _request;
		HttpResponse _response;
		ServerConfig _serverInfo;
		std::string _filePath;
		sockaddr_in _clientAddress;
		int _bytesReceived;
		std::string _body;

		std::vector< char * > _envp;
		std::vector< char * > _argv;
		std::vector< std::string > _envStrings;

		// Pipe handling
		int _pipefd[ 2 ];
		int _inputPipe[ 2 ];
		int _outputPipe[ 2 ];
		pid_t _pid;

		void buildEnvStrings();
		void doDup();
		void handleChildProcess();
		void updateStatus();
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