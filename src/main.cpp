// #include "Config/Debug.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"

class ServerConfig;
// Namespace to use on TCPServer
using namespace http;

std::vector<TcpServer*> initialize_all_servers(const Configs& configuration) {

	std::vector<TcpServer*> servers;

	for (size_t i = 0; i < configuration.servers.size(); ++i) {
		TcpServer* serv = new TcpServer(configuration.servers[i]);

		serv->runServer();
		servers.push_back(serv);
	}

	return servers;
};

std::vector<pollfd> get_all_listening_sockets_from(const std::vector<TcpServer*>& servers) {
	for (size_t i = 0; i < servers.size(); i++) {
		delete servers.at(i);
	}
};

void free_servers_memory(const std::vector<TcpServer*>& servers) {
}

int main(int ac, char** av) {

	Configs configuration;

	try {

		if (ac != 2)
			throw std::invalid_argument("Invalid number of arguments");

		if (CheckConf::checkConfExtension(av[1]) == false)
			throw std::invalid_argument("Extension has to be '.conf'");

		if (ReadConfig::setConfigs(av[1], configuration) == false)
			throw std::invalid_argument("Failed to set the configs");

	} catch (const std::exception& exception) {
		std::cout << "Failed to initialize servers: " << exception.what() << std::endl;
		return (-1);
	}

	std::vector<TcpServer*> servers = initialize_all_servers(configuration);
	std::vector<pollfd> all_servers_fds = get_all_listening_sockets_from(servers);
	ClientEventProcessor handleEvents(all_servers_fds, servers);

	free_servers_memory(servers);

	return 0;
}
