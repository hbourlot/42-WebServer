#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// void sendRequest(const std::string &method, const std::string &path,
//                  const std::string &body = "") {
//   int sock = socket(AF_INET, SOCK_STREAM, 0);
//   if (sock < 0) {
//     perror("Socket creation failed");
//     return;
//   }

//   sockaddr_in serverAddr{};
//   serverAddr.sin_family = AF_INET;
//   serverAddr.sin_port = htons(8080); // Cambia el puerto si es necesario
//   inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

//   if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//     perror("Connection failed");
//     close(sock);
//     return;
//   }

//   std::ostringstream request;
//   request << method << " " << path << " HTTP/1.1\r\n"
//           << "Host: localhost\r\n";

//   if (!body.empty()) {
//     request << "Content-Type: application/json\r\n"
//             << "Content-Length: " << body.size() << "\r\n";
//   }

//   request << "Connection: close\r\n\r\n";

//   if (!body.empty()) {
//     request << body;
//   }

//   std::string requestStr = request.str();
//   send(sock, requestStr.c_str(), requestStr.size(), 0);

//   char buffer[4096];
//   int bytesReceived;
//   while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
//     buffer[bytesReceived] = '\0';
//     std::cout << buffer;
//   }

//   close(sock);
//   std::cout << "\n--- Request Finished ---\n\n";
// }

// int main() {
//   // GET test
//   sendRequest("GET", "/login.html");

//   // POST test
//   sendRequest("POST", "/login", R"({"username":"alice","password":"1234"})");

//   // DELETE test
//   sendRequest("DELETE", "/somefile.txt");

//   // Invalid method test
//   sendRequest("FOO", "/");

//   return 0;
// }

