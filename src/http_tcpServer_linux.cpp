#include "http_tcpServer_linux.hpp"

namespace http {

	TcpServer::TcpServer(std::string ip_address, int port)
		: m_ip_address(ip_address), m_port(port), m_socket(), m_new_socket(),
		  m_incomingMessage(), m_socketAddress(),
		  m_socketAddress_len(sizeof(m_socketAddress)),
		  m_serverMessage("") { // Initialize m_serverMessage properly
		this->startServer();
	}

	TcpServer::~TcpServer() {
		close(m_socket);
		close(m_new_socket);
		// exit(1); // Exit with a failure code
	}

	int TcpServer::startServer() {
		m_socket = socket(AF_INET, SOCK_STREAM, 0); // Creates a socket
		if (m_socket < 0) {
			exitWithError("Cannot create socket");
			return 1;
		}

		// Set the socket address struct
		m_socketAddress.sin_family = AF_INET;
		m_socketAddress.sin_addr.s_addr =
			INADDR_ANY; // can replace this with a specific IP address if needed
		m_socketAddress.sin_port = htons(m_port);

		if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) <
			0) {
			exitWithError("Cannot bind socket to address");
			return 1;
		}

		return 0;
	}

	void TcpServer::shutDownServer() {
		close(m_socket);
		close(m_new_socket);
		// exit(0); // Exit with failure code??
	}

	void TcpServer::startListen() {
		if (listen(m_socket, 20) < 0) {
			exitWithError("Socket Listen failed");
		}

		std::ostringstream ss; // Output string stream for logging
		ss << "\n*** Listening on ADDRESS: "
		   << inet_ntoa(m_socketAddress.sin_addr)
		   << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
		log(ss.str());
	}

	void TcpServer::acceptConnection(SOCKET &new_socket) {
		new_socket = accept(m_socket, (sockaddr *)&m_socketAddress,
							&m_socketAddress_len);
		if (new_socket < 0) {
			std::ostringstream ss;
			ss << "Server failed to accept incoming connection from ADDRESS: "
			   << inet_ntoa(m_socketAddress.sin_addr)
			   << "; PORT: " << ntohs(m_socketAddress.sin_port);
			exitWithError(ss.str());
		} else {
			std::cout << "----- Connection Accepted" << std::endl;
		}
	}

	void TcpServer::readRequest() {
		char buffer[BUFFER_SIZE] = {0};

		bytesReceived = read(m_new_socket, buffer, BUFFER_SIZE - 1);
		if (bytesReceived < 0) {
			exitWithError("Failed to read bytes from client socket connection");
		}
		buffer[bytesReceived] = '\0';
	}

	void TcpServer::sendResponse() {
		m_serverMessage = "HTTP/1.1 200 OK\r\n"
						  "Content-Type: text/plain\r\n"
						  "Content-Length: 13\r\n"
						  "Connection: close\r\n"
						  "\r\n"
						  "Hello, world!";

		ssize_t bytesSent = send(m_new_socket, m_serverMessage.c_str(),
								 m_serverMessage.size(), 0);
		if (bytesSent < 0) {
			log("Error sending response to client");
		} else {
			log("----- Server Response sent to client -----\n\n");
		}
	}

	void TcpServer::runServer() {

		startListen();

		// SOCKET client_socket;
		// m_new_socket = client_socket;
		// acceptConnection(client_socket);

		acceptConnection(m_new_socket);

		readRequest();
		sendResponse();
		close(m_new_socket);
	}

} // namespace http