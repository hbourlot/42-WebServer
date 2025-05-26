#include "http_tcpServer_linux.hpp"
namespace http {

static std::string extractBoundary(httpRequest &request) {

  std::string contentType = request.headers["Content-Type"];
  std::string boundaryPrefix = "boundary=";

  size_t pos = contentType.find(boundaryPrefix);
  if (pos == std::string::npos) {
    return ("");
  }
  std::string boundary = "--" + contentType.substr(pos + boundaryPrefix.size());
  //   std::cout << boundary << std::endl;
  return (boundary);
}

static std::string extractFilePart(httpRequest &request,
                                   const std::string &boundary) {
  std::string body = request.body;

  size_t start = body.find(boundary);
  if (start == std::string::npos) {
    return ("");
  }
  start += boundary.length() + 2; // To skip the begin boundary and the /r/n
  //! need to verify if get line removed the /n
  size_t end = body.find(boundary, start);
  if (end == std::string::npos)
    end = body.size();

  return (body.substr(start, end - start));
}

bool splitHeadersAndContent(const std::string &filePart, std::string &headers,
                            std::string &content) {
  size_t headerEnd = filePart.find("\r\n\r\n");
  if (headerEnd == std::string::npos)
    return (false);
    // std::cerr << "\033[0;31m" <<  filePart << "\033[0m" <<  std::endl;

  headers = filePart.substr(0, headerEnd);
  content = filePart.substr(headerEnd + 4);
  return (true);
}

static std::string extractFilename(const std::string &headers) {
  std::string token = "filename=\"";
  size_t start = headers.find(token);
  if (start == std::string::npos)
    return ("");

  start += token.length();
  size_t end = headers.find("\"", start);
  if (end == std::string::npos)
    return ("");

  return (headers.substr(start, end - start));
}

static bool saveFile(const std::string &filename, const std::string &content) {
  std::string savePath = "./content/upload/" + filename;

  std::ofstream newfile(savePath.c_str(), std::ios::binary);
  if (!newfile.is_open())
    return (false);

  newfile << content;
  newfile.close();
  return (true);
}

bool TcpServer::parseMultipart() {
  //   printHttpHeaders(request);
  std::cout << request.body << std::endl;

  std::string boundary = extractBoundary(request);
  if (boundary.empty()) {
    setResponseError("400", "Bad Request: No boundary");
    return (false);
  }

  std::string filePart = extractFilePart(request, boundary);
  if (filePart.empty()) {
    setResponseError("400", "Bad Request: No boundary");
    return (false);
  }

  std::string headers;
  std::string content;
  if (!splitHeadersAndContent(filePart, headers, content)) {
    setResponseError("400", "Bad Request: Malformed multipart body");
    return (false);
  }

  std::string filename = extractFilename(headers);
  if (filename.empty()) {
    setResponseError("400", "Bad Request: Filename not found");
    return (false);
  }

  if (!saveFile(filename, content)) {
    setResponseError("500", "Internal Server Error: File not saved");
    return (false);
  }
  std::string msg = "File '" + filename + "' received";
  setResponse("200", "OK", "text/plain", msg);

  return (true);
}

} // namespace http
