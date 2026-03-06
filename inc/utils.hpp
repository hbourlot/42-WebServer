
#pragma once
#include "Config/Configs.hpp"
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

struct Directory;
struct File;

namespace http {
	class Request;
	class Response;
} // namespace http

std::string dateString();
time_t getActualTime();
std::string ft_strtrim(const std::string& str);
bool isDirectory(const std::string& filePath);
std::string joinPath(const std::string& base, const std::string& sub);
bool containBrackets(std::string& line, bool& state,
                     std::string extraWordToFind); // Track if the Server on config is open or closed;
void print(const char* src);

template <typename T> std::string ft_to_string(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
};

template <typename T> void p(T v) {
	std::cout << v << "\n";
}

// !DEBUG
void printHttpHeaders(const std::map<std::string, std::string>& headers);
void printLocation(const Directory& location);
bool checkSplitString(const std::string& line, const std::string& sep, bool& isServerOpen);

void PrintConfigs(Configs &configs);

std::string getFilePath(const http::Request& req, const ServerConfig& server);
const Directory* getMatchDirectory(const std::string& path, const std::vector<Directory>& locations);
const Location* getMatchLocation(const std::string& path, const std::vector<Location>& locations);

std::string createUploadBody();
ssize_t writeAll(int fd, const char* buf, size_t len);
void removeComment(std::string &line);
bool isDigits(std::string str);
