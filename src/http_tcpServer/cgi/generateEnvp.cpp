#include "http_tcpServer/Cgi.hpp"
#include <cstddef>
#include <string>
#include <vector>

// void Cgi::generateEnvp() {
// }
// for (size_t i = 0; _envp[i]; ++i) {
// 	envp.push_back(_envp[i]);
// }

// for (size_t i = 0; i < queryString.size(); ++i) {
// 	envp.push_back(const_cast<char *>(queryString[i].c_str()));
// }
// envp.push_back(NULL);

// void Cgi::generateEnvp() {
// 	std::vector<std::string> envStrings;

// 	envStrings.push_back("REQUEST_METHOD=" + method);
// 	envStrings.push_back("QUERY_STRING=" + join(queryString, "&"));
// 	envStrings.push_back("CONTENT_TYPE=" + contentType);
// 	envStrings.push_back("CONTENT_LENGTH=" + std::to_string(contentLength));
// 	envStrings.push_back("SCRIPT_NAME=" + scriptName);
// 	envStrings.push_back("SCRIPT_FILENAME=" + fileName);
// 	envStrings.push_back("SERVER_NAME=" + serverName);
// 	envStrings.push_back("SERVER_PORT=" + serverPort);
// 	envStrings.push_back("SERVER_PROTOCOL=" + serverProtocol);
// 	envStrings.push_back("SERVER_SOFTWARE=" + serverSoftware);
// 	envStrings.push_back("GATEWAY_INTERFACE=" + getWayInterface);
// 	envStrings.push_back("REMOTE_PORT=" + remotePort);
// 	envStrings.push_back("PATH_INFO=" + pathInfo);
// 	envStrings.push_back("PATH_TRANSLATED=" + pathTranslated);
// 	envStrings.push_back("REMOTE_ADDR=" + remoteAddr);

// 	// Convert HTTP headers to HTTP_* variables
// 	for (std::map<std::string, std::string>::const_iterator it = env.begin(); it
// != env.end(); ++it) { 		envStrings.push_back(it->first + "=" +
// it->second);
// 	}

// 	// Store char* versions
// 	for (size_t i = 0; i < envStrings.size(); ++i)
// 		envp.push_back(strdup(envStrings[i].c_str()));
// 	envp.push_back(NULL); // Null-terminate
// }

// std::string join(const std::vector<std::string>& v, const std::string& delim)
// { 	std::string result; 	for (size_t i = 0; i < v.size(); ++i) {
// result += v[i]; 		if (i + 1 != v.size()) 			result += delim;
// 	}
// 	return result;
// }

// for (std::map<std::string, std::string>::const_iterator it =
// _request.headers.begin();
//      it != _request.headers.end(); ++it) {
// 	std::string key = "HTTP_" + toUpperUnderscore(it->first);
// 	env[key] = it->second;
// }

// std::string toUpperUnderscore(std::string str) {
// 	for (size_t i = 0; i < str.length(); ++i) {
// 		if (str[i] == '-')
// 			str[i] = '_';
// 		elsew
// 			str[i] = std::toupper(str[i]);
// 	}
// 	return str;
// }