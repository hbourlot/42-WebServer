#include "server.hpp"
#include <sys/socket.h>

Server::Server(){
	this->socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

Server::~Server(){
}