#include "http_tcpServer_linux.hpp"

int main() {

	using namespace http;

	TcpServer server("127.0.0.1", 8080);

	server.runServer();

	return 0;
}