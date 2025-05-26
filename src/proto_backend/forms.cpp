#include "http_tcpServer/http_tcpServerException_linux.hpp"

protoBackend set_s_protoBackend(std::string statusCode, std::string statusMsg,
                                std::string htmlFilePath) {
  protoBackend result;
  result.statusCode = statusCode;
  result.statusMessage = statusMsg;
  result.htmlFilePath = htmlFilePath;
  return (result);
}

static std::map<std::string, std::string> parseForm(const std::string &body) {
  std::map<std::string, std::string> form;
  std::istringstream stream(body);
  std::string pair;
  while (std::getline(stream, pair, '&')) {
    ssize_t equal = pair.find('=');
    if (equal != std::string::npos) {
      std::string key = pair.substr(0, equal);
      std::string value = pair.substr(equal + 1);
      key = ft_strtrim(key);
      value = ft_strtrim(value);
      form[key] = value;
    }
  }
  return (form);
}

static protoBackend formurldeconded(httpRequest request) {
  protoBackend result;
  std::map<std::string, std::string> form = parseForm(request.body);
  if (form["username"] == "admin" && form["password"] == "1234") {
    // std::string htmlFilePath("./content/success.html");
    result = set_s_protoBackend("200", "OK", "");
  } else {
    // std::string errorFilePath("./content/error_401.html");
    result = set_s_protoBackend("401", "Unauthorized", "");
  }
  return (result);
}

protoBackend validateForm(httpRequest request) {
  protoBackend result;
  std::cout << request.headers["Content-Type"] << std::endl;
  if (request.headers["Content-Type"] == "application/x-www-form-urlencoded")
    result = formurldeconded(request);
  else if (request.headers["Content-Type"] == "application/json") {
    std::cout << "json form not valids for now" << std::endl;
    result = set_s_protoBackend("401", "Unauthorized", "");
  }
  return (result);
}
