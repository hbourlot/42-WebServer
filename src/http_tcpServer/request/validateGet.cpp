#include "http_tcpServer/http_tcpServer_linux.hpp"

namespace http {
static bool isDirectory(const std::string &filePath) {
  struct stat s;
  if (stat(filePath.c_str(), &s) != 0)
    return (false);
  return (S_ISDIR(s.st_mode));
}

static std::string getParentPath(const std::string &path) {
  std::string parent = path;

  if (!parent.empty() && parent[parent.length() - 1] == '/')
    parent.erase(parent.length() - 1);

  size_t prevfolder = parent.find_last_of('/');
  if (prevfolder != std::string::npos)
    return (parent.substr(0, prevfolder));

  return ("/");
}

static std::string joinPath(const std::string &base, const std::string &sub) {
  if (!base.empty() && base[base.length() - 1] == '/')
    return (base + sub);
  return (base + "/" + sub);
}

static std::string autoindex(std::string &dirPath, const Location *location,
                             httpRequest &request) {
  std::string autoindex = "<html>\n<body>\n<h1>Index of " + request.path + "</h1>\n";
  DIR *directory;

  directory = opendir(dirPath.c_str());
  if (!directory)
    return "<html><body><h1>Unable to open directory</h1></body></html>";

  struct dirent *dirent;
  dirent = readdir(directory);

  while (dirent != NULL) {
    std::string d_name(dirent->d_name);
    if (d_name.compare(".")) {

      if (d_name.compare(".."))

        autoindex += "<p><a href=" + joinPath(request.path, d_name) + ">" +
                     d_name + "</a></p>\n";
      else {

        std::string parentPath = getParentPath(request.path);
        autoindex += "<p><a href=" + parentPath + ">" + d_name + "</a></p>\n";
      }
    }
    dirent = readdir(directory);
  }
  autoindex = autoindex + "</body>\n</html>";
  closedir(directory);
  return (autoindex);
}

static std::string getFilePath(std::string &path, const Location *location) {

  std::string relativePath = path.substr(location->path.length());
  std::string filePath = location->root + "/" + relativePath;

  return (filePath);
}

static std::string getContentType(const std::string &path) {
  size_t dot = path.find_last_of('.');
  if (dot == std::string::npos)
    return "application/octet-stream"; // binario genérico

  std::string ext = path.substr(dot + 1);
  if (ext == "html" || ext == "htm")
    return "text/html";
  if (ext == "css")
    return "text/css";
  if (ext == "png")
    return "image/png";
  if (ext == "jpg" || ext == "jpeg")
    return "image/jpeg";
  if (ext == "gif")
    return "image/gif";
  if (ext == "txt")
    return "text/plain";
  if (ext == "pdf")
    return "application/pdf";
  return "application/octet-stream";
}

bool TcpServer::validateGet(const Location *location) {

  std::string filePath = getFilePath(request.path, location);

  if (isDirectory(filePath)) {
    if (!location->index.empty()) {

      std::string indexPath = filePath + location->index;
      std::ifstream indexFile(indexPath.c_str());

      if (indexFile.is_open()) {
        setHtmlResponse("200", "OK", indexPath);
        indexFile.close();
        return (true);
      }
    }

    if (!location->autoIndex) {

      setHtmlResponse("404", "Not Found", infos.errorPage[404]);
      return (false);
    } else {

      std::string body = autoindex(filePath, location, request);
      setResponse("200", "OK", "text/html", body);
      return (true);
    }
  }

  std::ifstream file(filePath.c_str(), std::ios::binary);

  if (!file.is_open()) {

    setHtmlResponse("404", "Not Found", infos.errorPage[404]);
    return (false);
  }

  std::stringstream body;
  body << file.rdbuf();
  file.close();
  setResponse("200", "OK", getContentType(filePath), body.str());

  //   setHtmlResponse("200", "OK", filePath);
  //   setResponse("200", "OK", "/text/plain", "OK");
  return (true);
}
} // namespace http
