#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

static std::map<std::string, std::string> parseForm(const std::string &body) {
  std::map<std::string, std::string> form;
  std::istringstream stream(body);
  std::string pair;
  while (std::getline(stream, pair, '&')) {
    ssize_t equal = pair.find('=');
    if (equal != std::string::npos) {
      std::string key = pair.substr(0, equal);
      std::string value = pair.substr(equal + 1);
      form[key] = value;
    }
  }
  return (form);
}

bool TcpServer::formurldeconded()
{
  std::map<std::string, std::string> form = parseForm(request.body);
  if (form["username"] == "admin" && form["password"] == "1234\n") {
    std::ifstream htmlFile("./content/success.html");
    if(setHtmlResponse("200", "OK", htmlFile) == false)
      return(false);
  } else {
    std::ifstream errorFile("./content/error_401.html");
    if(setHtmlResponse("401", "Unauthorized", errorFile) == false)
      return false;
  }
  return(true);
}

bool TcpServer::validateForm()
{
  std::cout << request.headers["Content-Type"] << std::endl; 
  if (request.headers["Content-Type"] ==
    "application/x-www-form-urlencoded")
    {
      if(formurldeconded()== false)
      return(false);
    }
  else if(request.headers["Content-Type"] == "application/json"){

    std::cout << "json form" << std::endl;
    std::ifstream errorFile("./content/error_401.html");
    if(setHtmlResponse("401", "Unauthorized", errorFile) == false)
    return(false); 
  }
  return(true);
}

bool TcpServer::validatePost() {
  if (request.path == "/login") {
    std::cout << request.body << std::endl;
    if(validateForm()== false)
      return(false);
    return(true);
  }
  setResponseError("404", "Not Found");
  return false;
}

} // namespace http
