#pragma once
#include "Configs.hpp"
#include <string>

struct ServerConfig;
struct Location;

std::string removeSpace(std::string &line);
std::string getInfo(std::string &noSpaceLine);
Location &findPath(ServerConfig server, std::string path);