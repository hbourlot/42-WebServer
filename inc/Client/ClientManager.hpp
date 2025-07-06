#pragma once

#include "Client.hpp"
#include <map>
#include <vector>

namespace http {

	class ClientManager {
	  public:
		ClientManager();
		~ClientManager();

		void addClient(SocketFD fd, sockaddr_in &socketAddress, std::vector<pollfd> &fds, const Server serverInfo);
		void removeClient(SocketFD);
		Client *getClient(SocketFD);
		bool hasClient(SocketFD) const;
		bool hasCgiClient(SocketFD, std::vector<Client> &) const;
		std::map<SocketFD, Client *> &getClients();

		void resetClientState(int fd);

	  private:
		std::map<SocketFD, Client *> _clients;
	};
} // namespace http
