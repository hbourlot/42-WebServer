#include "CGI/CgiHandler.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

// !!! [] FIX GET CONSTRUCTOR
http::CgiHandler::CgiHandler(Location &loc, httpRequest &request)
    : m_method("GET"), m_requestBody(request.body),
      m_contentLength(request.body.length()),
      m_contentType(request.headers["Content-Type"])
{
	std::cout << "--- Default CGI constructor 🟩" << std::endl;
	// Default constructor
}

http::CgiHandler::~CgiHandler()
{
}