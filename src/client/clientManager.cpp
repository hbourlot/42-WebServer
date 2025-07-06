#include "Client/ClientManager.hpp"
#include <netinet/in.h>
#include <sys/poll.h>
#include <vector>

namespace http {

	ClientManager::ClientManager() {
	}
	ClientManager::~ClientManager() {
		std::map<int, Client *>::iterator it;
		for (it = _clients.begin(); it != _clients.end(); ++it) {
			delete it->second;
		}
		_clients.clear();
	}

	void ClientManager::addClient(SocketFD fd, sockaddr_in &socketAddress, std::vector<pollfd> &fds,
	                              const Server serverInfo) {
		if (_clients.find(fd) != _clients.end()) {
			std::cerr << "Client with FD " << fd << " already exists." << std::endl;
			return;
		}
		_clients[fd] = new Client(fd, socketAddress, fds, serverInfo);
	}

	void ClientManager::removeClient(SocketFD fd) {
		std::map<int, Client *>::iterator it = _clients.find(fd);
		if (it != _clients.end()) {
			delete it->second;
			_clients.erase(it);
		}
	}
	Client *ClientManager::getClient(SocketFD fd) {
		std::map<int, Client *>::iterator it = _clients.find(fd);
		if (it != _clients.end()) {
			return it->second;
		}
		return NULL;
	}
	bool ClientManager::hasClient(SocketFD fd) const {
		return _clients.find(fd) != _clients.end();
	}

	bool ClientManager::hasCgiClient(SocketFD fd, std::vector<Client> &clients) const {

		// for (int idx = 0; idx < clients.size(); ++idx) {
		// 	if (clients[idx].getCgi())
		// }
		return true;
	}

	std::map<int, Client *> &ClientManager::getClients() {
		return _clients;
	}

	void ClientManager::resetClientState(int fd) {
		if (!hasClient(fd))
			return;
		Client *client = _clients[fd];
		client->clearBuffers();
		client->resetRequest();
		client->resetResponse();
		client->setCgiInProgress(false);
		// client->setRequestComplete(false);
	}

} // namespace http
