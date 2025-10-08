#pragma once

#include "Cgi.hpp"
#include "Client/ClientManager.hpp"
#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include "HttpHandler.hpp"
#include "HttpLogs.hpp"
#include "HttpStatus.hpp"
#include "HttpStructs.hpp"
#include "HttpUtils.hpp"
#include "ResponseBuilder.hpp"
#include "Upload/UploadManager.hpp"
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
		TcpServer(ServerConfig server);
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
		ServerConfig _serverInfo;
		SocketFD _serverSocket;
		//! Still figuring where put it on  std::set<SocketFD> _toBeClosed;
		ClientManager _clientManager;

		std::map<SocketFD, sockaddr_in> _socketAddressMap;
		unsigned int _socketAddress_len;
		std::vector<Cgi> _cgi;
		std::map<int, Cgi *> _cgiFdMap;

		int startServer();
		void runLoop(int timeOut);
		void shutDownServer();
		void startListen();
		void acceptConnection();
		void removeDeadConnections();
		void processClientEvents();
		void closeClientConnection(size_t index);
		bool readRequest(int index);

		bool handleCgiResponse(pollfd &socket);
		int sendResponse(pollfd &socket);

		bool parseCgi(const Location loc, std::string &filePath, sockaddr_in &clientAddress, httpRequest &request);
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations, const std::string &field);

} // namespace http

// bool parseMultipart(const Location &location);
// bool handleDirectoryListing(const std::string &filePath, const Location &location);