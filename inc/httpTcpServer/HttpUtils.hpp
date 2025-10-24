#pragma once

#include "../Config/Configs.hpp"
#include "HttpStructs.hpp"

std::string ft_strtrim(const std::string &str);
void printHttpHeaders(const httpRequest &request);

bool isDirectory(const std::string &filePath);
std::string getFilePath(std::string &path, const Location &location);
std::string joinPath(const std::string &base, const std::string &sub);
std::vector<std::string> split(const std::string &s, char delimiter);

std::string getContentType(const std::string &path);

void handleDirectoryListing(Client &client, const ServerConfig &server, const std::string &filePath,
                            const Location &location);

std::string readFileContent(const std::string &filePath);
std::string joinPath(const std::string &base, const std::string &sub);
std::vector<std::string> split(const std::string &s, char delimiter);

ParseStatus parseRequest(httpRequest &request, const std::string &requestContent, const ServerConfig &serverInfo,
                         size_t maxBodySize);

void printLocation(const Location &location);
