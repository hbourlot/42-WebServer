#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

void TcpServer::setResponseError(std::string statusCode,
                                 std::string statusMsg) {
  std::string body = statusMsg + " (" + statusCode + ")";
  std::ostringstream response;
  response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
           << "Content-Type: text/plain\r\n"
           << "Content-Length: " << body.length() << "\r\n"
           << "Connection: Close\r\n"
           << "\r\n"
           << body;
  m_serverMessage = response.str();
  log(response.str());
}

bool TcpServer::validateGet() {
  std::cout << request.path << std::endl;
  if (request.path == "/")
    request.path = "login.html";
  std::cout << request.path << std::endl;
  std::string fullPath = "./content/" + request.path;
  std::ifstream htmlFile(fullPath.c_str());
  if (!htmlFile.is_open()) {
    setResponseError("404", "Not Found");
    return false; // TODO: throw error would be better
  }

  // Read the HTML file into a string
  std::ostringstream buffer;
  buffer << htmlFile.rdbuf();
  std::string fileContent = buffer.str();
  htmlFile.close();

  std::ostringstream response;
  response << "HTTP/1.1 200 OK\r\n"
           << "Content-type: text/html\r\n"
           << "Content-Length: " << fileContent.size() << "\r\n"
           << "Connection: close\r\n"
           << "\r\n"
           << fileContent;

  m_serverMessage = response.str();
  return (true);
}

bool TcpServer::validateRequest() {
  if (request.method == "GET") {
    if (validateGet() == false) {
      return (false);
    }
  } else if (request.method == "POST")
    std::cout << "Suposed to do something" << std::endl;
  else if (request.method == "DELETE")
    std::cout << "Suposed to do something" << std::endl;
  else {
    setResponseError("405", "Method Not Allowed");
    return false;
  }
  return true;
}

} // namespace http
