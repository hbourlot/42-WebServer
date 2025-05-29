#include "http_tcpServer/http_tcpServer_linux.hpp"

namespace http {
static bool isDirectory(const std::string &filePath) {
  struct stat s;
  if (stat(filePath.c_str(), &s) != 0)
  return (false);
  return (S_ISDIR(s.st_mode));
}

static std::string autoindex(std::string &dirPath, const Location *location, httpRequest &request)
{
    std::string autoindex= "<htlm>\n<body>\n<h1>Index of " + request.path + "</h1>\n";
    DIR *directory;
    directory=opendir(dirPath.c_str());
    struct dirent* dirent;
    dirent = readdir(directory);
    while(dirent != NULL)
    {
        std::string d_name(dirent->d_name);
        if(d_name.compare(".")){
          if(d_name.compare(".."))
          autoindex += "<p><a href=" + location->path +"/"+ d_name+ ">" +d_name + "</a></p>\n";
          else
          autoindex += "<p><a href=./.. >" +d_name + "</a></p>\n"; // !Still failing in case of /files/css/ href=./.. Need search!!!
        }
        dirent = readdir(directory);
    }
    autoindex = autoindex + "</body>\n</html>";
    closedir(directory);
    return(autoindex);
}

static std::string getFilePath(std::string &path, const Location *location) {

  std::string relativePath = path.substr(location->path.length());
  std::string filePath = "." + location->root + "/" + relativePath;

  // !Just test . for now
  // std::cout << "path " << path << std::endl;
  // std::cout << "relativepath " << relativePath << std::endl;
  // std::cout << "filepath " << filePath << std::endl;
  return (filePath);
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
      return(true);
    }
  }

  std::ifstream file(filePath.c_str());

  if (!file.is_open()) {

    setHtmlResponse("404", "Not Found", infos.errorPage[404]);
    return (false);
  }
  file.close();
  setHtmlResponse("200", "OK", filePath);
  return (true);
}
} // namespace http
