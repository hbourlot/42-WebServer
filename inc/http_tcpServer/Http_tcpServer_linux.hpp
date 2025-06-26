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
#include <map>
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

class Cgi;

namespace http {

	typedef int SocketFD;
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
		Server _serverInfo;
		std::string m_ip_address;
		pollfd _currentClient;
		int _port, _bytesReceived, _bytesSend;
		SocketFD _serverSocket;
		long _incomingMessage;
		std::map<SocketFD, sockaddr_in> _socketAddressMap;
		unsigned int _socketAddress_len;
		std::string _serverMessage;
		std::vector<Cgi> _cgi;
		std::map<int, Cgi *> _cgiFdMap;

		int startServer();
		void runLoop(std::vector<pollfd> &fds, int timeOut);
		void shutDownServer(std::vector<pollfd> &fds);
		void setCurrentClient(std::vector<pollfd> &client);

		void startListen();
		void acceptConnection(std::vector<pollfd> &fds);
		void readRequest(std::vector<pollfd> &fds, int i);
		bool handleRequest(pollfd &socket, std::vector<pollfd> &fds,
		                   sockaddr_in &clientAddress);
		bool handleGetRequest(const Location &location,
		                      sockaddr_in &clientAddress);
		bool handlePostRequest(const Location &location);
		bool handleDeleteRequest(const Location &location);
		bool handleCgiResponse(pollfd &socket);
		int sendResponse(pollfd &socket);

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

		void clearResponse();
		void processClientEvents(std::vector<pollfd> &fds);

		bool parseCgi(const Location loc, std::string &filePath,
		              sockaddr_in &clientAddress, httpRequest &request);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations,
	                                     const std::string &field);

} // namespace http
