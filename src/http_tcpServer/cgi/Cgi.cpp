#include "http_tcpServer/Cgi.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>

static void buildArgv(std::vector<char *> &object, const char *filePath) {

	object.push_back(const_cast<char *>(filePath));
	object.push_back(nullptr);
}

static void buildEnvp(std::vector<char *> &envp,
                      const std::map<std::string, std::string> &cgiVars) {
	std::vector<std::string> envStrings;
	for (std::map<std::string, std::string>::const_iterator it =
	         cgiVars.begin();
	     it != cgiVars.end(); ++it) {
		envStrings.push_back(it->first + "=" + it->second);
	}
	for (size_t i = 0; i < envStrings.size(); ++i) {
		envp.push_back(const_cast<char *>(envStrings[i].c_str()));
	}
	envp.push_back(NULL);
}

// _method(), _queryString(), _contentType(), _fileName(),
//       _scriptName(), _serverName(), _serverPort(), _serverSoftware(),
//       _serverProtocol(), _getWayInterface(), _remotePort(), _pathInfo(),
//       _pathTranslated(), _httpUserAgent(), _acceptLanguage(), _cookie(),
//       _referer(), _filePath(), _envp(), _argv()

http::Cgi::Cgi(httpRequest &request, std::string &filePath,
               sockaddr_in &clientAddress)
    : _request(request), _filePath(filePath.c_str()),
      _clientAddress(clientAddress) {

	// Cgi::createValidCgiExtensions();

	std::map<std::string, std::string> cgiVars;

	// execve
	this->_filePath = const_cast<const char *>(filePath.c_str());
	buildArgv(this->_argv, _filePath);
	// buildEnvp(_envp, const std::map<std::string, std::string> &cgiVars)
}

// ENVP

// method,
// queryString,
// contentType,
// fileName,
// scriptName,
// serverName,
// serverPort,
// serverProtocol,
// serverSoftware,
// getWayInterface,
// remotePort,
// pathTranslated,
// httpUserAgent,
// acceptLanguage,
// cookie,
// referer,

// END

http::Cgi::~Cgi() {
}