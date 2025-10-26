#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <netinet/in.h>
#include <sys/poll.h>

bool http::TcpServer::handleCgiResponse(pollfd &socket)
{
	Client *client = _clientManager.getClient(socket.fd);

	std::cout << "[DEBUG] FD: " << socket.fd << " | revents: " << socket.revents << " | events: " << socket.events
	          << std::endl;
	if ((socket.revents & POLLIN) && _cgiFdMap.count(socket.fd))
	{
		std::cout << "FD ON HANDLE => " << socket.fd << std::endl;
		Cgi *cgi = _cgiFdMap[socket.fd];
		cgi->readCgiOutput();

		client->getResponse() = ResponseBuilder::buildResponse(HTTP_OK, cgi->getBody());

		std::cout << cgi->getBody();
		sendResponse(socket);
		_cgiFdMap.erase(socket.fd);
		return true;
	}
	return false;
}

void http::TcpServer::closeClientConnection(size_t index)
{
	SocketFD fd = _fds[index].fd;

	// std::cout << "Closing client FD => " << fd << std::endl;
	std::string msg("Closing FD => ");
	msg += to_str(fd);

	Logs::log(ERROR, msg);

	close(fd);
	_socketAddressMap.erase(fd);
	_clientManager.removeClient(fd);
	_fds.erase(_fds.begin() + index);
}

void http::TcpServer::processClientEvents()
{

	SocketFD fd;
	bool shouldCloseSend;
	bool shouldCloseRead;
	sockaddr_in *currentAddress;

	for (size_t idx = 1; idx < _fds.size(); ++idx)
	{
		shouldCloseSend = false;
		shouldCloseRead = false;

		fd = _fds[idx].fd;

		if (_fds[idx].revents & POLLIN)
		{
			shouldCloseRead = readRequest(idx);
		}
		if (_fds[idx].revents & POLLOUT)
		{
			int sendResult = sendResponse(_fds[idx]);
			if (sendResult == 1)
				shouldCloseSend = true;
			else if (sendResult == 2)
				_fds[idx].events |= POLLOUT;
			else
				_fds[idx].events &= ~POLLOUT;
		}
		if (shouldCloseRead || shouldCloseSend)
		{
			closeClientConnection(idx);
			--idx;
			continue;
		}
		// _fds[idx].events |= POLLIN;
	}
}