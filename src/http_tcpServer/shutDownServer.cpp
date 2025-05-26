#include "http_tcpServer/http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::shutDownServer(std::vector<pollfd> &fds) {
		for (int i = 0; i < fds.size(); ++i) {
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
		}
	}
} // namespace http