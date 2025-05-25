#include "http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::shutDownServer() {
		close(m_serverSocket);
		close(m_acceptSocket);
		// exit(0); // Exit with failure code??
	}
} // namespace http