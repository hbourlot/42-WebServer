#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http {

bool TcpServer::handlePostRequest(const Location *location) {

  if (request.path == "/login") {
    httpResponse result = validateForm(request);

    if (!result.htmlFilePath.empty()) {
      setHtmlResponse(result.statusCode, result.statusMessage,
                      result.htmlFilePath);
    } else
      setResponseError(result.statusCode, result.statusMessage);
  } else if (request.path == "/upload") {
    if (!location->uploadEnable) {
      setResponseError("403", "Upload Not Allowed");
      return (false);
    }
    parseMultipart(location);
  } else {

    setHtmlResponse("404", "Not Found", infos.errorPage[404]);
    return (false);
  }
  return true;
}

} // namespace http
