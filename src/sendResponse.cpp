#include "http_tcpServer_linux.hpp"

namespace http {
void TcpServer::sendResponse() {
  ssize_t bytesSent = send(m_new_socket, m_serverMessage.c_str(), m_serverMessage.size(), 0);
  if (bytesSent < 0) {
    log("Error sending response to client");
  } else {
    log("----- Server Response sent to client -----\n\n");
  }
}
} // namespace http
