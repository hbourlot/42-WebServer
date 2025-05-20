#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

bool TcpServer::validateGet() {
  if (request.path == "/")
    request.path = "login.html";
  // std::cout << request.path << std::endl;
  std::string fullPath = "./content/" + request.path;
  std::ifstream htmlFile(fullPath.c_str());
  std::ifstream errorFile("./content/error_404.html");
  if (!htmlFile.is_open()) {
    if(!errorFile.is_open())
      setResponseError("404", "Not Found");
  setHtmlResponse("404","Not Found", errorFile);
    return false; // TODO: throw error would be better
  }
  setHtmlResponse("200","OK",htmlFile);
  return (true);
}

bool TcpServer::validatePost()
{
  if(request.path == "/login")
    std::cout << request.body << std::endl;
    
  return(true);
}

bool TcpServer::validateRequestMethod() {
  if (request.method == "GET") {
    if (validateGet() == false)
      return (false);
  } else if (request.method == "POST")
  {
    // std::cout << "Suposed to do something" << std::endl;
    validatePost();
  }
  else if (request.method == "DELETE")
    std::cout << "Suposed to do something" << std::endl;
  else {
    setResponseError("405", "Method Not Allowed");
    return false;
  }
  return true;
}
}