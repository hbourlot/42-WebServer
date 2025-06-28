#pragma once

#include "Cgi.hpp"
#include "Client/ClientManager.hpp"
#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include "HttpLogs.hpp"
#include "HttpStatus.hpp"
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

namespace http
{

	const int BUFFER_SIZE = 30720;

	class TcpServer
	{
	  public:
		// Default Constructor
		TcpServer(Server server);
		// Default Destructor
		~TcpServer();

		// Main member
		int runServer();

		class TcpServerException : public std::runtime_error
		{
		  public:
			explicit TcpServerException(const std::string &message) : std::runtime_error(message)
			{
			}
		};

	  private:
		// *Setted inside a server must know the fds it handles, and less passing by parameter
		std::vector<pollfd> _fds;
		Server _serverInfo;
		SocketFD _serverSocket;
		ClientManager _clients;

		httpRequest _request;
		httpResponse _response;
		std::string _serverMessage;
		std::string _ipAddress;
		int _port;
		std::map<SocketFD, sockaddr_in> _socketAddressMap;
		unsigned int _socketAddress_len;
		std::vector<Cgi> _cgi;
		std::map<int, Cgi *> _cgiFdMap;

		int startServer();
		void runLoop(int timeOut);
		void shutDownServer();
		void startListen();
		void acceptConnection();

		// std::map<int, clientState> _clients; // ! Maybe its better

		bool readRequest(std::vector<pollfd> &fds, int i);
		void closeClient(std::vector<pollfd> &fds, size_t &i);
		bool handleRequest(pollfd &socket, std::vector<pollfd> &fds, sockaddr_in &clientAddress);
		bool handleGetRequest(const Location &location, sockaddr_in &clientAddress);
		bool handlePostRequest(const Location &location);
		bool handleDeleteRequest(const Location &location);
		bool handleCgiResponse(pollfd &socket);
		int sendResponse(pollfd &socket);

		void prepareResponse(const HttpStatusCode &status, const std::string &body, const std::string &headerKey = "",
		                     const std::string &headerValue = "");
		void setResponse();
		void setBodyResponse(const HttpStatusCode &status, const std::string &body,
		                     const std::string &contentType = "text/plain");
		void setFileResponse(const HttpStatusCode &status, const std::string &filePath, bool isError = false);
		void setRedirect(const HttpStatusCode &status, std::string redirection);
		void setResponseError(const HttpStatusCode &status);

		bool parseMultipart(const Location &location);
		bool handleDirectoryListing(const std::string &filePath, const Location &location);

		void clearResponse();
		void processClientEvents(std::vector<pollfd> &fds);

		bool parseCgi(const Location loc, std::string &filePath, sockaddr_in &clientAddress, httpRequest &request);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations, const std::string &field);

} // namespace http
