#pragma once

#include "Client.hpp"
#include <map>

class ClientManager
{
  public:
	ClientManager();
	~ClientManager();

	void addClient(int fd);
	void removeClient(int fd);
	Client *getClient(int fd);
	bool hasClient(int fd) const;
	std::map<int, Client *> &getClients();

  private:
	std::map<int, Client *> _clients;
};
