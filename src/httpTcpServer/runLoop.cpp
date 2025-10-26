#include "httpTcpServer/HttpStructs.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <cstddef>
#include <exception>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>

// Remove and close all pollfd's with HUP, ERR, or NVAL events
void http::TcpServer::removeDeadConnections()
{

	for (size_t i = 1; i < _fds.size(); ++i)
	{
		if (_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			std::cout << "--- Removing CONNECTION\n";

			SocketFD fd = _fds[i].fd;

			if (_socketAddressMap.count(fd))
				_socketAddressMap.erase(fd);

			std::string msg("Closing FD => ");
			msg += to_str(fd);

			Logs::log(ERROR, msg);
			close(fd);

			_clientManager.removeClient(fd);

			_fds.erase(_fds.begin() + i);
			--i;
		}
	}
}

void http::TcpServer::runLoop(int timeOut)
{

	try
	{
		while (true)
		{
			// poll() waits for events on multiple file descriptors (like
			// sockets), enabling non-blocking I/O in servers.
			int ret = poll(_fds.data(), _fds.size(), timeOut);

			if (ret < 0)
			{
				std::cerr << "poll() failed" << std::endl;
				shutDownServer();
			}
			else if (ret == 0)
			{
				std::cerr << "poll() timeOut. Closing Server." << std::endl;
				shutDownServer();
				return;
			}

			// Checking for new connections
			acceptConnection();
			removeDeadConnections();
			processClientEvents();
		}
	}
	catch (const TcpServerException &e)
	{
		std::cerr << "Error handling client connection => " << e.what() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[EXCEPTION] std::exception: " << e.what() << std::endl;
	}
}
