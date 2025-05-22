#include "http_tcpServer_linux.hpp"


namespace http {

bool TcpServer::validatePost() {
  if (request.path == "/login") {
    std::cout << request.body << std::endl;
    protoBackend result;
    result = validateForm(request);
    if(!result.htmlFilePath.empty())
    {
      std::ifstream htmlFile(result.htmlFilePath.c_str());
      setHtmlResponse(result.statusCode, result.statusMessage, htmlFile);     
    }
    else
     setResponseError(result.statusCode, result.statusMessage);
  }
  else if(request.path == "/upload")
  {
  }
  return false;
}

} // namespace http
