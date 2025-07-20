#pragma once

#include <netinet/in.h>
#include <poll.h>

// #include "http_tcpServer_linux/HttpStructs.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/HttpStructs.hpp"
#include "http_tcpServer/ICgi.hpp"
#include <string>
#include <sys/poll.h>
#include <vector>

namespace http {

	class Client {
	  public:
		// TODO: [] Maybe i wont need clientSocket for Cgi
		Client(SocketFD fd, sockaddr_in &socketAddress, std::vector<pollfd> &fds, const Server serverInfo);

		~Client();

		SocketFD getFd() const;
		bool hasCgi() const;
		ICgi *getCgi() const;
		std::vector<pollfd> &getFdsLoop();
		void addCgi(ICgi *);
		void executeCgi() const;

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
		void setPOLLOUT();

		bool isCgiInProgress() const;
		void setCgiInProgress(bool value);

		bool parseCgi(const Location loc, std::string &filePath);

	  private:
		int _fd;

		std::string _readBuffer;
		std::string _writeBuffer;
		sockaddr_in &_socketAddress;
		ICgi *_cgi;

		httpRequest _request;
		httpResponse _response;
		std::vector<pollfd> &_fds;
		Server _serverInfo;

		bool _requestComplete;
		bool _cgiInProgress;
	};

}; // namespace http
