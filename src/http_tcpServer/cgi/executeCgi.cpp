#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <iostream>
#include <unistd.h>

// void handleCgiParentProcess(Cgi &object) {

// if (object.inputPipe[0] != -1)
// 	close(object.inputPipe[0]);
// if (object.outputPipe[1] != -1)
// 	close(object.outputPipe[1]);

// char buffer[http::BUFFER_SIZE + 1] = {0};
// std::string requestContent;
// int bytesReceived = 0;

// while ((bytesReceived =
//             read(object.outputPipe[0], buffer, http::BUFFER_SIZE)) > 0) {
// 	requestContent.append(buffer, bytesReceived);
// }

// if (bytesReceived < 0) {
// 	if (errno != EAGAIN && errno != EWOULDBLOCK) {
// 		std::cerr << "Error: read()\n";
// 		//! ... handle error properly
// 		return;
// 	}
// }

// setResponse
// }

static void doDup(int (&inputPipe)[2], int (&outputPipe)[2]) {
	dup2(inputPipe[0], STDIN_FILENO);
	dup2(outputPipe[1], STDOUT_FILENO);

	close(inputPipe[1]);
	close(inputPipe[0]);
	close(outputPipe[0]);
	close(outputPipe[1]);
}

namespace http {

	void TcpServer::executeCgi(Cgi &object) {

		// if (pipe(object.inputPipe) == ERROR ||
		//     pipe(object.outputPipe) == ERROR) {
		// 	// ! Handle error
		// 	// send error message to the client.
		// 	return;
		// }
		// object.pid = fork();
		// if (object.pid < 0) {
		// 	// Handle error here
		// 	// Send error response to the client.
		// 	return;
		// } else if (object.pid == 0) {

		// 	// Child process
		// 	doDup(object.inputPipe, object.outputPipe);
		// 	execve(object.filePath, object.argv.data(), object.envp.data());
		// 	// If execve fails
		// 	_exit(1);
		// } else {

		// 	// handleCgiParentProcess(object);

		// 	if (object.inputPipe[0] != -1)
		// 		close(object.inputPipe[0]);
		// 	if (object.outputPipe[1] != -1)
		// 		close(object.outputPipe[1]);

		// 	char buffer[http::BUFFER_SIZE + 1] = {0};
		// 	std::string requestContent;

		// 	read(object.outputPipe[0], buffer, BUFFER_SIZE);
		// 	std::cout << buffer;
		// 	// while ((_bytesReceived =
		// 	//             read(object.outputPipe[0], buffer, BUFFER_SIZE)) > 0)
		// 	//             {
		// 	// 	requestContent.append(buffer, _bytesReceived);
		// 	// }

		// 	if (_bytesReceived < 0) {
		// 		if (errno != EAGAIN && errno != EWOULDBLOCK) {

		// 			std::cerr << "Error: read()\n";
		// 			//! ... handle error properly
		// 			return;
		// 		}
		// 	}
		// 	setBodyResponse("200", "OK", buffer);
		// 	std::cout << m_serverMessage;
		// }
	}

} // namespace http