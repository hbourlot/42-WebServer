#pragma once

#include "CGI/CgiHandler.hpp"
#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define DFL_404 "content/defaults/error_404.html"
#define DFL_405 "content/defaults/error_405.html"
#define DFL_500 "content/defaults/error_500.html"

struct httpRequest
{
	std::string method;
	std::string path;
	std::string protocol;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> queries;
};

struct httpResponse
{
	std::string statusCode;
	std::string statusMessage;
	std::string htmlFilePath;
	std::map<std::string, std::string> headers;
};

namespace
{

	void log(const std::string &message)
	{
		std::cout << message << std::endl;
	}

	void exitWithError(const std::string &errorMessage)
	{
		log("ERROR: " + errorMessage);
		exit(1); // Use exit(1) to indicate an error
	}

	void logDebugger(const std::string &message)
	{
		std::cout << "Debugger => " << message << std::endl;
	}

} // namespace

namespace http
{

	typedef int HTTP_SOCKET;
	const int BUFFER_SIZE = 30720;

	class TcpServer
	{
	  public:
		// Default Constructor
		TcpServer(Configs configuration);
		// Default Destructor
		~TcpServer();

		// Main member
		int runServer();

		class TcpServerException : public std::runtime_error
		{
		  public:
			explicit TcpServerException(const std::string &message)
			    : std::runtime_error(message)
			{
			}
		};

	  private:
		httpRequest request;
		Server infos; // For keep infos inside
		std::string m_ip_address;
		int m_port, bytesReceived, bytesSend;
		HTTP_SOCKET m_serverSocket, m_acceptSocket;
		long m_incomingMessage;
		struct sockaddr_in m_socketAddress;
		unsigned int m_socketAddress_len;
		std::string m_serverMessage;
		std::vector<class CgiHandler> CGI;

		int startServer();
		void runLoop(std::vector<pollfd> &fds, int timeOut);
		void shutDownServer(std::vector<pollfd> &fds);
		void startListen();
		void acceptConnection(std::vector<pollfd> &fds);
		void readRequest(std::vector<pollfd> &fds, int i);
		bool validateRequest();
		bool handleGetRequest(const Location *location, httpRequest &request);
		bool handlePostRequest(const Location *location);
		bool handleDeleteRequest(const Location *location);
		int sendResponse(pollfd socket);
		void setResponseError(std::string statusCode, std::string statusMsg);
		bool setHtmlResponse(std::string statusCode, std::string statusMsg,
		                     const std::string &htmlFilePath);
		bool parseMultipart(const Location *location);
		void setResponse(std::string statusCode, std::string statusMsg,
		                 std::string contentType, std::string body);
		void addCgi(const Location &loc, const httpRequest &request);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations,
	                                     const std::string &field);

} // namespace http

//*request
void parseRequest(httpRequest &request, const std::string &requestContent);
//*Lib
std::string ft_strtrim(const std::string &str);
//*prot_backend forms
httpResponse validateForm(httpRequest request);
//* utils
void printHttpHeaders(const httpRequest &request);

bool isDirectory(const std::string &filePath);
std::string getFilePath(std::string &path, const Location *location);
std::string joinPath(const std::string &base, const std::string &sub);
std::vector<std::string> split(const std::string &s, char delimiter);
