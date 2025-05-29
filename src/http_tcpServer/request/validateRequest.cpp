#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>

namespace http {

const Location *getMatchLocation(const std::string &path,
                                 const std::vector<Location> &locations) {

  const Location *matchedLocation = NULL;
  size_t matchLength = 0;

  for (size_t i = 0; i < locations.size(); ++i) {

    const std::string &locPath = locations[i].path;

    if (path.compare(0, locPath.size(), locPath) == 0 &&
        locPath.size() > matchLength) {
      matchedLocation = &locations[i];
      matchLength = locPath.size();
    }
  }
  return (matchedLocation);
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

  const Location *matchedLocation =
      getMatchLocation(request.path, infos.locations);

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