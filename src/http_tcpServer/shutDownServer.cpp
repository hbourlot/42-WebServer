#include "http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::shutDownServer() {
		close(m_socket);
		close(m_acceptSocket);
		// exit(0); // Exit with failure code??
	}
} // namespace http