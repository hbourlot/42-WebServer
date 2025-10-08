#pragma once

#include "Client.hpp"
#include <map>

namespace http {
    class TcpServer; 
}
class ClientManager
{
  public:
	ClientManager();
	~ClientManager();

	void addClient(int fd, http::TcpServer &server);
	void removeClient(int fd);
	Client *getClient(int fd);
	bool hasClient(int fd) const;
	std::map<int, Client *> &getClients();

	void resetClientState(int fd);

  private:
	std::map<int, Client *> _clients;
};
