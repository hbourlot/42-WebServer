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
    //   value.erase(0, value.find_last_not_of(" \t\r\n"));
      form[key] = value;
    }
  }
  return (form);
}

// bool TcpServer::validatePost() {
//   if (request.path == "/login") {

//     //   std::cout << request.body << std::endl;
//     std::map<std::string, std::string> form = parseForm(request.body);
//     if (form["username"] == "admin" && form["password"] == "1234") {
//       std::ifstream htmlFile("./content/success.html");
//       if (!htmlFile.is_open()) {
//         std::ifstream errorFile("./content/error_404.html");
//         if (!errorFile.is_open()) {
//           setResponseError("404", "Not Found");
//           return (false);
//         }
//         std::cout << "html post" << std::endl;
//         setHtmlResponse("200", "OK", htmlFile);
//       }
//       //   setHtmlResponse("200", "OK", )
//       setResponseError("401", "Unathorized");
//     }
//   }
//   return (true);
// }
bool TcpServer::validatePost() {
  if (request.path == "/login") {
    std::cout << request.body << std::endl;
    std::map<std::string, std::string> form = parseForm(request.body);
    std::cout << (form["username"] == "admin") << std::endl;
    std::cout << (form["password"] == "1234\n" )<< std::endl;
    if (form["username"] == "admin" && form["password"] == "1234\n") {
      std::ifstream htmlFile("./content/success.html");
      if (!htmlFile.is_open()) {
        setResponseError("404", "Not Found");
        return false;
      }
      setHtmlResponse("200", "OK", htmlFile);
      return true;
    } else {
      std::ifstream errorFile("./content/error_401.html");
      if (!errorFile.is_open()) {
        setResponseError("401", "Unauthorized");
        return false;
      }
      setHtmlResponse("401", "Unauthorized", errorFile);
      return false;
    }
  }

  setResponseError("404", "Not Found"); // Si la ruta no es /login
  return false;
}

} // namespace http
