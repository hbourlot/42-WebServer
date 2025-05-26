#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>

namespace http {

bool TcpServer::validateGet() {
  if (request.path == "/")
    request.path = "login.html";
  // std::cout << request.path << std::endl;
  std::string fullPath = "./content/" + request.path;
  std::ifstream htmlFile(fullPath.c_str());
  if (setHtmlResponse("200", "OK", htmlFile) == false)
    return false; // TODO: throw error would be better
  return (true);
}

bool TcpServer::validateRequestMethod() {
  std::cerr << request.method << std::endl;
  if (request.method == "GET") {
    if (validateGet() == false)
      return (false);
  } else if (request.method == "POST") {
    validatePost();
  } else if (request.method == "DELETE")
    std::cout << "Suposed to do something" << std::endl;
  else {
    std::cerr << "\033[0;31m" <<  request.method << "\033[0m" <<  std::endl;

    setResponseError("405", "Method Not Allowed");
    return false;
  }
  return true;
}
} // namespace http