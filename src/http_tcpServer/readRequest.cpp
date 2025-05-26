#include "http_tcpServer/http_tcpServer_linux.hpp"
#include <sys/poll.h>

static void parseRequest(httpRequest &request,
                         const std::string &requestContent) {
  std::istringstream request_stream(requestContent);
  std::string line;
  size_t idx;
  std::getline(request_stream, line);
  std::istringstream first_line(line);

  first_line >> request.method >> request.path >> request.protocol;
  while (std::getline(request_stream, line)) {
    if (line == "\r" || line == "")
      break;
    idx = line.find(":");
    if (idx != std::string::npos) {
      std::string key = line.substr(0, idx);
      std::string value = line.substr(idx + 1);
      key = ft_strtrim(key);
      value = ft_strtrim(value);
      request.headers[key] = value;
    }
  }
  // std::cout << requestContent << std::endl;
  std::string body;
  while (std::getline(request_stream, line))
    body += line + "\n";
  request.body = body;
  // request.body = request_stream.str();
  //  std::cerr << "\033[0;31m" <<  request.body << "\033[0m" <<  std::endl;
}

void http::TcpServer::readRequest(std::vector<pollfd> &fds, int i) {
	char buffer[BUFFER_SIZE + 1] = {0};

	bytesReceived = read(fds[i].fd, buffer, BUFFER_SIZE);
	if (bytesReceived < 0) {
		std::cerr << "Error: read()\n";
		close(fds[i].fd);
		fds.erase(fds.begin() + i);
		return;
		// throw TcpServerException(
		// 	"Failed to read bytes from client socket connection");
	} else if (bytesReceived == 0) {
		return;
	}
	buffer[bytesReceived] = '\0';
	write(2, buffer, BUFFER_SIZE);
	std::string requestContent("");
  for(int i=0; i < bytesReceived; i++)
   requestContent += buffer[i]; 
  // std::cout << requestContent << std::endl;
	parseRequest(request, requestContent);

  // Set event POLLOUT
  fds[i].events |= POLLOUT;
}
