#pragma once

#include "http_tcpServerException_linux.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

namespace {

	void log(const std::string &message) {
		std::cout << message << std::endl;
	}

	void exitWithError(const std::string &errorMessage) {
		log("ERROR: " + errorMessage);
		exit(1); // Use exit(1) to indicate an error
	}

} // namespace

namespace http {

	typedef int SOCKET;
	const int BUFFER_SIZE = 30720;

	class TcpServer {
		public:
			TcpServer(std::string ip_address, int port);
			~TcpServer();
			void runServer();

		private:
			// MAYBE YOU STRUCT HERE?? 😇
			std::string m_ip_address;
			int m_port, m_socket, m_new_socket, bytesReceived, bytesSend;
			long m_incomingMessage;
			struct sockaddr_in m_socketAddress;
			unsigned int m_socketAddress_len;
			std::string m_serverMessage;

			int startServer();
			void shutDownServer();
			void startListen();
			void acceptConnection(SOCKET &new_socket);
			void readRequest();
			void sendResponse();
	};

} // namespace http