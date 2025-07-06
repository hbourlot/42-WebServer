#pragma once

#include "Client.hpp"
#include <map>

namespace http {

	class ClientManager {
	  public:
		ClientManager();
		~ClientManager();

		void addClient(SocketFD fd, sockaddr_in &socketAddress, pollfd &clientSocket, const Server serverInfo);
		void removeClient(SocketFD fd);
		Client *getClient(SocketFD fd);
		bool hasClient(SocketFD fd) const;
		std::map<int, Client *> &getClients();

		void resetClientState(int fd);

	  private:
		std::map<SocketFD, Client *> _clients;
	};
} // namespace http
