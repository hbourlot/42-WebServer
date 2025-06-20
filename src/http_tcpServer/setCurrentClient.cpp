#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <vector>

// void http::TcpServer::setCurrentClient(pollfd &client) {
// 	_currentClient.fd = client.fd;
// 	_currentClient.events = client.events;
// 	_currentClient.revents = client.revents;
// }