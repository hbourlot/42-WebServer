#pragma once
#include "Configs.hpp"
#include <string>

struct ServerConfig;
struct Directory;

std::string removeSpace( std::string &line );
bool getMaxRequestBody(std::string &noSpaceLine, size_t &result);
std::string getInfo( std::string &noSpaceLine );
Directory &findPath( ServerConfig server, std::string path );