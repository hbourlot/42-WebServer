#include "http_tcpServer/http_tcpServer_linux.hpp"

namespace http {

bool TcpServer::validatePost() {

  if (request.path == "/login") {
    httpResponse result = validateForm(request);

    if (!result.htmlFilePath.empty()) {
      setHtmlResponse(result.statusCode, result.statusMessage,
                      result.htmlFilePath);
    } else
      setResponseError(result.statusCode, result.statusMessage);
  } else if (request.path == "/upload") {
    parseMultipart();
  }
  return false;
}

} // namespace http
