#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>

namespace http {

bool TcpServer::validateGet(const Location *location) {

  // std::string fullPath = "." + location->root + "/" + "index.html"
  std::string indexPath = "." + location->root + "/" /* + location->index */;
  // !Just test . for now
  std::ifstream indexFile(indexPath.c_str());

  if (!indexFile.is_open()) {
    if (!location->autoIndex) {
      setHtmlResponse("404", "Not Found", infos.errorPage[404]);
      return (false);
    }
    // !Here implement if autoindex its enabled
  }
  indexFile.close();
  setHtmlResponse("200", "OK", indexPath);
  return (true);
}

static bool validateRequestMethod(httpRequest request,
                                  const Location *location) {

  if (request.method != "GET" && request.method != "POST" &&
      request.method != "DELETE") {
    return false;
  }

  for (size_t i = 0; i < location->methods.size(); ++i) {
    if (request.method == location->methods[i])
      return true;
  }
  return false;
}

bool TcpServer::validateRequest() {

  const Location *matchedLocation = NULL;
  for (size_t i = 0; i < infos.locations.size(); ++i) {
    if (request.path == infos.locations[i].path) {
      matchedLocation = &infos.locations[i];
      break;
    }
  }

  if (!matchedLocation) {
    setHtmlResponse("404", "Not Found", infos.errorPage[404]);
    return false;
  }
  //! Treat before the /redirect-me
  if (validateRequestMethod(request, matchedLocation) == false) {
    setHtmlResponse("405", "Method Not Allowed", DFL_405);
    return (false);
  }

  if (request.method == "GET")
    return (validateGet(matchedLocation));
  else if (request.method == "POST")
    return (validatePost());
  else if (request.method == "DELETE") {
    // Here still mising implement the part of Method Delete
  }
  return (true);
}
} // namespace http