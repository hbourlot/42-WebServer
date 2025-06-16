#pragma once


#include <cstdint>
#include <ctime>
#include <fstream>
#include <sstream>

#include "CGI/CgiHandler.hpp"
#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
#include "HttpLogs.hpp"
#include "HttpStruct.hpp"
#include "HttpUtils.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <poll.h>
#include <sstream>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define DFL_404 "content/defaults/error_404.html"
#define DFL_405 "content/defaults/error_405.html"
#define DFL_500 "content/defaults/error_500.html"

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
		httpResponse response;
		Server infos; // For keep infos inside
		std::string m_ip_address;
		int m_port, bytesReceived, bytesSend;
		HTTP_SOCKET m_serverSocket, m_acceptSocket;
		long m_incomingMessage;
		struct sockaddr_in m_socketAddress;
		unsigned int m_socketAddress_len;
		std::string m_serverMessage;

		int startServer();
		void runLoop(std::vector<pollfd> &fds, int timeOut);
		void shutDownServer(std::vector<pollfd> &fds);
		void startListen();
		void acceptConnection(std::vector<pollfd> &fds);
		void readRequest(std::vector<pollfd> &fds, int i);
		bool validateRequest();
		bool handleGetRequest(const Location &location);
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
	};

	std::string getLocationFieldAsString(const std::vector<Location> &locations,
	                                     const std::string &field);

} // namespace http
