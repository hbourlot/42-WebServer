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
		// exit(1); //TODO Exit with a failure code??
	}

	int TcpServer::startServer() {

		// 							domain, type, protocol
		// Creates a server socket (IPv4, TCP, 0);
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socket < 0) {
			throw TcpServerException("Cannot create socket");
			return 1;
		}

		// Set the socket address struct
		m_socketAddress.sin_family = AF_INET;
		m_socketAddress.sin_addr.s_addr =
			INADDR_ANY; // can replace this with a specific IP address if needed
		m_socketAddress.sin_port =
			htons(m_port); // Converts 16-bit integer in host byte order

		// Associate socket with a specific IP addr and Port number (sockfd,
		// sockaddr *, addrlen)
		if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) <
			0) {
			throw TcpServerException("Cannot bind socket to address");
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
			throw TcpServerException("Socket Listen failed");
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
			ss << "Server failed to accept incoming connection from =>\n"
				  "[ADDRESS: "
			   << inet_ntoa(m_socketAddress.sin_addr) << "]\n"
			   << "[PORT: " << ntohs(m_socketAddress.sin_port) << "]\n";
			throw TcpServerException(ss.str());
		} else {
			std::cout << "----- Connection Accepted 🟩" << std::endl;
		}
	}

	void TcpServer::readRequest() {
		char requestContent[BUFFER_SIZE] = {0};

		bytesReceived = read(m_new_socket, requestContent, BUFFER_SIZE - 1);
		if (bytesReceived < 0) {
			throw TcpServerException(
				"Failed to read bytes from client socket connection");
		}
		requestContent[bytesReceived] = '\0';
		write(1, requestContent, BUFFER_SIZE);
	}

	void TcpServer::sendResponse() {

		// Need to select which response will be sent

		// m_serverMessage = "HTTP/1.1 200 OK\r\n"
		// 				  "Content-Type: text/plain\r\n"
		// 				  "Content-Length: 13\r\n"
		// 				  "Connection: close\r\n"
		// 				  "\r\n"
		// 				  "Hello, world!";

		ssize_t bytesSent = send(m_new_socket, m_serverMessage.c_str(),
								 m_serverMessage.size(), 0);
		if (bytesSent < 0) {
			log("Error sending response to client");
		} else {
			log("----- Server Response sent to client -----\n\n");
		}
	}

	void TcpServer::runServer() {

		try {
			startListen();
		} catch (const TcpServerException &e) {
			std::cerr << "Error while starting to listen => " << e.what()
					  << std::endl;
			return;
		}

		// while (true) {
		try {
			acceptConnection(m_new_socket);
			readRequest();
			std::string requestContent = "login.html";
			validateRequest(requestContent);
			sendResponse();

		} catch (const TcpServerException &e) {
			std::cerr << "Error handling client connection => " << e.what()
					  << std::endl;
		}
		// }

		shutDownServer();
		// SOCKET client_socket;
		// m_new_socket = client_socket;
		// acceptConnection(client_socket);
	}

} // namespace http