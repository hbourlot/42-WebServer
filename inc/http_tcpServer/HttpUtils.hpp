#pragma once

#include "HttpStructs.hpp"

void parseRequest(httpRequest &request, const std::string &requestContent);
std::string ft_strtrim(const std::string &str);
httpResponse validateForm(httpRequest request);
void printHttpHeaders(const httpRequest &request);

bool isDirectory(const std::string &filePath);
std::string getFilePath(std::string &path, const Location &location);
std::string joinPath(const std::string &base, const std::string &sub);
std::vector<std::string> split(const std::string &s, char delimiter);

std::string getContentType(const std::string &path);

bool handleDirectoryListing(const std::string &filePath,
                            const Location *location,
                            const httpRequest &request, httpResponse &response);

std::string readFileContent(const std::string &filePath)