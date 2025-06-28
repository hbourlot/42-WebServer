#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 2058

int main() {

	int upd_socket;
	sockaddr_in server_addr;
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	char buffer[BUFFER_SIZE];

	std::string response = "Hello MDF!";

	// 1 create a UDP socket
	upd_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (upd_socket < 0) {
		std::cerr << "Socket creation failed";
		return 1;
	}

	// 2. Bind to IP and Port
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(upd_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Bind failed";
		return 1;
	}

	std::cout << "UPD Server listening on port " << PORT << "..." << std::endl;

	while (true) {
		memset(buffer, 0, BUFFER_SIZE);

		ssize_t bytes_received =
			recvfrom(upd_socket, buffer, BUFFER_SIZE - 1, 0,
					 (sockaddr *)&client_addr, &client_len);
		if (bytes_received < 0) {
			std::cerr << "recvfrom failed";
			continue;
		}

		buffer[bytes_received] = '\0';

		// 4. send response back
		ssize_t b_s = sendto(upd_socket, response.c_str(), response.size(), 0,
							 (sockaddr *)&client_addr, client_len);
		if (b_s < 0) {
			std::cerr << "sendto failed";
		} else {
			std::cout << "Response send to client." << std::endl;
		}
	}

	close(upd_socket);
	return 0;
}
