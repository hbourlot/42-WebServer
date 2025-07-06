#pragma once

#include <netinet/in.h>
#include <poll.h>

// #include "http_tcpServer_linux/HttpStructs.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/HttpStructs.hpp"
#include <string>
#include <sys/poll.h>

namespace http {

	class Client {
	  public:
		// TODO: [] Maybe i wont need clientSocket for Cgi
		Client(int fd, sockaddr_in &socketAddress, pollfd &clientSocket, const Server serverInfo);

		// Prev version
		Client(int fd, sockaddr_in &socketAddress, pollfd &clientSocket);
		~Client();

		SocketFD getFd() const;

		// Buffers
		std::string &getReadBuffer();
		std::string &getWriteBuffer();
		void appendToReadBuffer(const std::string &data);
		void appendToWriteBuffer(const std::string &data);
		void clearBuffers();
		void clearReadBuffer();
		void clearWriteBuffer();

		// request-response structures
		httpRequest &getRequest();
		httpResponse &getResponse();
		void resetRequest();
		void resetResponse();

		// State of CGi REquest
		bool isRequestComplete() const;
		void setRequestComplete(bool value);

		bool isCgiInProgress() const;
		void setCgiInProgress(bool value);

		bool parseCgi(const Location loc, std::string &filePath);

	  private:
		int _fd;

		std::string _readBuffer;
		std::string _writeBuffer;
		sockaddr_in &_socketAddress;

		httpRequest _request;
		httpResponse _response;
		pollfd &_clientSocket;
		Server _serverInfo;

		bool _requestComplete;
		bool _cgiInProgress;
	};

}; // namespace http
