// #include "Config/Debug.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"

class ServerConfig;

void cleanup(int signo) {
	getStopServer() = true;
}

bool &getStopServer() {
	static bool s_stopServer = false;
	return (s_stopServer);
}

int main(int argc, char **argv) {

	struct sigaction sa;
	sa.sa_handler = cleanup;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	Configs configuration;
	try {

		if (argc != 2)
			throw std::invalid_argument("Invalid number of arguments");

		if (CheckConf::checkConfExtension(argv[1]) == false)
			throw std::invalid_argument("Extension has to be '.conf'");

		// Creates a config object
		// Reads the configuration file
		if (ReadConfig::setConfigs(argv[1], configuration) == false) {
			std::cout << "Failed to set the configs" << std::endl;
			exit(-1);
		}
	} catch (const std::exception &exception) {
		std::cout << "Failed to set the configurations: " << exception.what() << std::endl;
		return (-1); // If we use exit, we got a leak from exception
	}

	// Namespace to use on TCPServer
	using namespace http;

	std::vector<TcpServer> servers;

	// for (size_t i = 0; i < configuration.servers.size(); i++) {
	TcpServer server(configuration.servers[0]);
	server.runServer();
	// servers.push_back(server);
	// }

	// for (size_t i = 0; i < servers.size(); i++) {

	// 	std::cout <<	configuration.servers.size() << std::endl;
	// 	servers.at(i).runServer();
	// }

	return 0;
}
