#include "httpTcpServer/HttpTcpServerLinux.hpp"

namespace http
{
	void TcpServer::shutDownServer()
	{
		for (int i = 0; i < _fds.size(); ++i)
		{
			close(_fds[i].fd);
			_fds.erase(_fds.begin() + i);
		}
	}
} // namespace http