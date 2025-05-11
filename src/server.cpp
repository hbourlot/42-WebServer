#include "server.hpp"
#include <sys/socket.h>

Server::Server(){
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		// Criates a socket (comunication point) so receive and send information;
		// AF_INET -> Designate the type of address to IPV4
		// SOCK_STREAM -> Means that is a conection socket based on TCP
		// 0 -> The system choose the protocol for SOCK_STREAM 

	this->_serverAddress.sin_family = AF_INET; // Checks the ip family
	this->_serverAddress.sin_port = htons(8080); // Converts the number of port to network byte order, for system compatibility
	this->_serverAddress.sin_addr.s_addr = INADDR_ANY; // Accept interface connections of (localHost, extern Ip, etc)
}

Server::~Server(){
}

void Server::bindServer(){
	bind(_serverSocket, (struct sockaddr*)& _serverAddress, sizeof(_serverAddress));
	listen(_serverSocket, 5);
}