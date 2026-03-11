#pragma once

#include "Cgi.hpp"
#include "HttpTcpServerLinux.hpp"
#include "Session/SessionManager.hpp"
#include "utils.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>
#include <vector>

typedef int SocketFD;

namespace http {

	class EventProcessor {
	  public:
		class ClientEventProcessorException : public std::runtime_error {
		  public:
			explicit ClientEventProcessorException(const std::string& msg) : std::runtime_error(msg){};
		};

		EventProcessor(std::vector<pollfd>& allServerFds, std::vector<TcpServer*> servers);

		~EventProcessor();

		void run();

		void acceptConnections();

		bool removeDeadConnections(size_t& index);

		void shutDownProcessor();

		void processRead(pollfd& pfd, Client* client, Cgi* cgi);

		bool processWrite(pollfd& pfd, Client* client, int index);

		bool processClientEvents(int index);

		void registerCgi(http::Cgi* cgi);

		void cleanupCgi(http::Cgi* cgi);

		// bool hasCgiSuccessfullyFinished(Cgi* cgi) const;

		// GETTERS

		SessionManager& getSessionManager();

	  private:
		std::vector<pollfd>& _allSockets;
		std::vector<struct sockaddr_in> _socketAddress;
		size_t _serverSocketSize;
		std::vector<TcpServer*> _servers;
		ClientManager _clientManager;
		SessionManager _sessionManager;
		std::map<SocketFD, Cgi*> _cgi_by_fd;

		size_t _clientIndex;

		bool closeIdleConnection(size_t index);
		bool handleClientIndex(size_t& index);

		void setSession(Client* client);

		void closeClientConnection(size_t index);

		void handleCgiIO(Client* client);

		bool handleResponse(pollfd& pfd, Client& client);

		void processRequest(Client& client);
	};
} // namespace http