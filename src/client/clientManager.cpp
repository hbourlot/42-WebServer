#include "Client/ClientManager.hpp"

ClientManager::ClientManager() {
}
ClientManager::~ClientManager() {
	std::map<int, Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it) {
		delete it->second;
	}
	_clients.clear();
}

void ClientManager::removeAllClients() {
	// Iterate through all clients and delete them
	for (std::map<int, Client*>::iterator it = _clients.begin(); 
		 it != _clients.end(); ++it) {
		if (it->second) {
			delete it->second;  // Free each Client object
			it->second = NULL;
		}
	}
	_clients.clear();  // Clear the map
	std::cout << "ClientManager: Removed all clients (" << _clients.size() << ")" << std::endl;
}

void ClientManager::addClient(int fd, http::TcpServer &server) {
	if (_clients.find(fd) != _clients.end()) {
		std::cerr << "Client with FD " << fd << " already exists." << std::endl;
		return;
	}
	_clients[fd] = new Client(fd, server);
}

void ClientManager::removeClient(int fd) {
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	}
}

Client *ClientManager::getClient(int fd) {
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end()) {
		return it->second;
	}
	return NULL;
}
bool ClientManager::hasClient(int fd) const {
	return _clients.find(fd) != _clients.end();
}

std::map<int, Client *> &ClientManager::getClients() {
	return _clients;
}

void ClientManager::resetClientState(int fd) {
	Client *client = getClient(fd);

	if (!client)
		return;

	client->setState(RESET);
	client->clearBuffers();
	client->resetRequest();
	client->resetResponse();
	client->setCgiInProgress(false);
	// client->setRequestComplete(false);
}
