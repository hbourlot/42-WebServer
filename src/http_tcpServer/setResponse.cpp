#include "http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace http {

	void TcpServer::setResponseError(std::string statusCode,
									 std::string statusMsg) {
		std::string body = statusMsg + " (" + statusCode + ")";
		std::ostringstream response;
		response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
				 << "Content-Type: text/plain\r\n"
				 << "Content-Length: " << body.length() << "\r\n"
				 << "Connection: close\r\n"
				 << "\r\n"
				 << body;
		m_serverMessage = response.str();
		log(response.str());
	}

bool TcpServer::setHtmlResponse(std::string statusCode, std::string statusMsg ,std::ifstream &htmlFile) {
  
  if(!htmlFile.is_open())
  {
    std::ifstream errorFile("./content/error_404.html");
    if (!errorFile.is_open())
      setResponseError("404", "Not Found");
     else 
    setHtmlResponse("404", "Not Found", errorFile);
    return(false);
  }
  // Read the HTML file into a string
  std::ostringstream buffer;
  buffer << htmlFile.rdbuf();
  std::string fileContent = buffer.str();
  htmlFile.close();
  std::ostringstream response;
  response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
  << "Content-type: text/html\r\n"
  << "Content-Length: " << fileContent.size() << "\r\n"
  << "Connection: close\r\n"
  << "\r\n"
  << fileContent;
  
  m_serverMessage = response.str();
  

  std::string log_str = "HTTP/1.1 " + statusCode + " " + statusMsg + "\r\n";
  log(log_str);
  return(true);
	}
} // namespace http
