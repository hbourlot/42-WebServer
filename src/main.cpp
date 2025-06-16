// #include "Config/Debug.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

int main(int argc, char **argv)
{

	if (argc != 2)
		throw std::invalid_argument("Error: Invalid number of arguments");

	using namespace http;

	Configs configuration; // Creates a config object
	ReadConfig::setConfigs(argv[1],
	                       configuration); // Reads the configuration file

	// std::vector<TcpServer> servers;

	// configuration.server[0];
	// configuration.server[1];

	// servers[0] = configuration.server[0]
	// servers[1] = configuration.server[1]

	// for (int i = 0; i < configuration.servers.size(); i++){
	// 	TcpServer server(configuration.servers[i]);
	// 	servers.push_back(server);
	// }

	TcpServer server(configuration);

	server.runServer();

	return 0;
}
