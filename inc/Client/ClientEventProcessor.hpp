#pragma once

#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>

namespace http {

	class ClientEventProcessor {
	  public:
		friend class Router;

		ClientEventProcessor(std::vector<pollfd>& allServerFds, std::vector<TcpServer*> servers);

		ClientEventProcessor(TcpServer& server);

		~ClientEventProcessor();

		void run();

		void acceptConnections();

		bool removeDeadConnections(size_t& index);

		void processRead(pollfd& pfd, Client* client, Cgi* cgi);

		void processWrite(pollfd& pfd, Client* client, int index);

		void processClientEvents(int index);

		void registerCgi(http::Cgi* cgi);

		void cleanupCgi(http::Cgi* cgi);

		bool hasCgiFinished(Cgi* cgi) const;

		bool hasCgiSuccessfullyFinished(Cgi* cgi) const;

	  private:
		std::vector<pollfd>& _allSockets;
		std::vector<struct sockaddr_in> _socketAddress;
		std::vector<TcpServer*> _servers;
		ClientManager _clientManager;

		TcpServer _server; // ! will remove
		size_t _clientIndex;

		bool readFromSocket(SocketFD fd, std::string& readBuffer, IN_OUT_STATE& state);

		bool parseRequestData(Client& client, const ServerConfig& serverInfo);

		bool sendResponse(pollfd& pfd, Client& client);

		bool handleResponse(pollfd& pfd, Client& client);

		void closeConnection(size_t index);

		bool processRequest(Client& client);

		bool buildErrorResponse(Client& client, IN_OUT_STATE state);

		bool handleRouteValidation(Client& client, VALIDATION_STATUS& validationStatus);
	};
} // namespace http