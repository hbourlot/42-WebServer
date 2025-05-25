#pragma once

#include "CheckConfName.hpp"
#include "ReadConfig.hpp"

#include "http_tcpServerException_linux.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <ostream>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct httpRequest {
		std::string method;
		std::string path;
		std::string protocol;
		std::map<std::string, std::string> headers;
		std::string body;
};

namespace {

	void log(const std::string &message) {
		std::cout << message << std::endl;
	}

	void exitWithError(const std::string &errorMessage) {
		log("ERROR: " + errorMessage);
		exit(1); // Use exit(1) to indicate an error
	}

	void logDebugger(const std::string &message) {
		std::cout << "Debugger => " << message << std::endl;
	}

} // namespace

namespace http {

	typedef int HTTP_SOCKET;
	const int BUFFER_SIZE = 30720;

	class TcpServer {
		public:
			// Default Constructor
			TcpServer(std::string ip_address, int port);
			// Default Destructor
			~TcpServer();
			void runServer();

		private:
			// MAYBE YOU STRUCT HERE?? 😇
			httpRequest request;
			std::string m_ip_address;
			int m_port, bytesReceived, bytesSend;
			HTTP_SOCKET m_serverSocket, m_acceptSocket;
			long m_incomingMessage;
			struct sockaddr_in m_socketAddress;
			unsigned int m_socketAddress_len;
			std::string m_serverMessage;

			int startServer();
			void shutDownServer();
			void startListen();
			void acceptConnection(std::vector<pollfd> &fds);
			void readRequest(std::vector<pollfd> &fds, int i);
			bool validateRequestMethod();
			bool validateGet();
			void setResponseError(std::string statusCode,
								  std::string statusMsg);
			void setHtmlResponse(std::ifstream &htmlFile);
			int sendResponse(pollfd socket);
	};

} // namespace http
