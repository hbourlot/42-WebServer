#include "http_tcpServer_linux.hpp"

int main() {

	using namespace http;

	TcpServer server("0.0.0.0", 8080);

	server.runServer();

	return 0;
}