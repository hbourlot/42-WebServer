#pragma once

#include "Cgi.hpp"
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
#include <netinet/in.h>
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
		httpRequest _request;
		httpResponse _response;
		Server _infos;
		std::string m_ip_address;
		pollfd _currentClient;
		int m_port, _bytesReceived, _bytesSend;
		HTTP_SOCKET _serverSocket, _acceptSocket;
		long _incomingMessage;
		std::vector<sockaddr_in> _socketAddress;
		unsigned int _socketAddress_len;
		std::string _serverMessage;
		std::vector<Cgi> m_cgi;

		int startServer();
		void runLoop(std::vector<pollfd> &fds, int timeOut);
		void shutDownServer(std::vector<pollfd> &fds);
		void setCurrentClient(std::vector<pollfd> &client);

		void startListen();
		void acceptConnection(std::vector<pollfd> &fds);
		void readRequest(std::vector<pollfd> &fds, int i);
		bool handleRequest(sockaddr_in &clientAddress);
		bool handleGetRequest(const Location &location,
		                      sockaddr_in &clientAddress);
		bool handlePostRequest(const Location &location);
		bool handleDeleteRequest(const Location &location);
		int sendResponse(pollfd socket);

		void setResponse();
		void setBodyResponse(const std::string &statusCode,
		                     const std::string &statusMsg,
		                     const std::string &body,
		                     const std::string &contentType = "text/plain");
		void setFileResponse(std::string statusCode, std::string statusMsg,
		                     const std::string &htmlFilePath,
		                     bool isError = false);

		bool parseMultipart(const Location &location);
		bool handleDirectoryListing(const std::string &filePath,
		                            const Location &location);
		void setResponseError(std::string statusCode, std::string statusMsg);
		bool parseMultipart(const Location *location);

		void clearResponse(httpRequest &request, std::string &serverMessage);
		void processClientEvents(std::vector<pollfd> &fds);

		bool parseCgi(const Location loc, std::string &filePath,
		              sockaddr_in &clientAddress, httpRequest &request);
		void executeCgi(Cgi &object);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations,
	                                     const std::string &field);

} // namespace http
