// #include "Config/Debug.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

int main(int argc, char **argv, char **envp)
{

	if (argc != 2)
		throw std::invalid_argument("Error: Invalid number of arguments");

	using namespace http;

	// Creates a config object
	Configs configuration;
	
	// Reads the configuration file
	ReadConfig::setConfigs(argv[1], configuration);

	TcpServer server(configuration.servers[0]);

	server.runServer();

	return 0;
}
