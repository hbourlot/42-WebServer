#pragma once

#include "Client.hpp"
#include <map>
#include <vector>

namespace http {

	struct Client;

	class ClientManager {
	  public:
		ClientManager();
		~ClientManager();

		void addClient(SocketFD fd, sockaddr_in &socketAddress, std::vector<pollfd> &fds, const Server serverInfo);
		void removeClient(SocketFD);
		Client *getClient(SocketFD);
		bool hasClient(SocketFD) const;
		bool hasCgiClient(int fd) const;
		std::map<SocketFD, Client *> &getClients();

		void resetClientState(int fd);

		void addCgi(int fd, Client *);
		Client *getCgiClient(int fd);
		void removeCgi(int fd);
		void removeCgiByClientFd(int clientFd);

	  private:
		std::map<SocketFD, Client *> _clients;
		std::map<int, Client *> _cgiToClient;
	};
} // namespace http
