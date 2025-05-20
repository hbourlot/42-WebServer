#include "http_tcpServer_linux.hpp"

namespace http {

	void TcpServer::shutDownServer() {
		close(m_socket);
		close(m_new_socket);
		// exit(0); // Exit with failure code??
	}
} // namespace http