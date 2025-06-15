#pragma once

#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include "HttpLogs.hpp"
#include "HttpStructs.hpp"
#include "HttpUtils.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <poll.h>
#include <set>
#include <sstream>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define DFL_404 "content/defaults/error_404.html"
#define DFL_405 "content/defaults/error_405.html"
#define DFL_500 "content/defaults/error_500.html"

#ifndef nullptr
#define nullptr NULL
#endif

#define ERROR -1

namespace http {

	typedef int HTTP_SOCKET;
	const int BUFFER_SIZE = 30720;

	class TcpServer {
	  public:
		// Default Constructor
		TcpServer(Configs configuration);
		// Default Destructor
		~TcpServer();

		// Main member
		int runServer();

		class TcpServerException : public std::runtime_error {
		  public:
			explicit TcpServerException(const std::string &message)
			    : std::runtime_error(message) {
			}
		};

	  private:
		httpRequest request;
		httpResponse response;
		Server infos; // For keep infos inside
		std::string m_ip_address;
		int m_port, bytesReceived, bytesSend;
		HTTP_SOCKET m_serverSocket, m_acceptSocket;
		long m_incomingMessage;
		struct sockaddr_in m_socketAddress;
		unsigned int m_socketAddress_len;
		std::string m_serverMessage;
		std::vector<Cgi> m_cgi;

		int startServer();
		void runLoop(std::vector<pollfd> &fds, int timeOut);
		void shutDownServer(std::vector<pollfd> &fds);
		void startListen();
		void acceptConnection(std::vector<pollfd> &fds);
		void readRequest(std::vector<pollfd> &fds, int i);
		bool handleRequest();
		bool handleGetRequest(const Location *location);
		bool handlePostRequest(const Location *location);
		bool handleDeleteRequest(const Location *location);
		int sendResponse(pollfd socket);
		void setResponse();
		void setResponseError(std::string statusCode, std::string statusMsg);
		bool setHtmlResponse(std::string statusCode, std::string statusMsg,
		                     const std::string &htmlFilePath);
		bool parseMultipart(const Location *location);
		void setResponse(std::string statusCode, std::string statusMsg,
		                 std::string contentType, std::string body, int len);
		void clearResponse(httpRequest &request, std::string &serverMessage);
		void processClientEvents(std::vector<pollfd> &fds);

		// CGI
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
		bool parseCgi(const Location loc, std::string &filePath);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations,
	                                     const std::string &field);

} // namespace http
