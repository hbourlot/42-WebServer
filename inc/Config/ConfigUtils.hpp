#pragma once
#include "Configs.hpp"
#include <string>

struct ServerConfig;
struct Directory;

std::string removeSpace( std::string &line );
int getMaxRequestBody(std::string &noSpaceLine);
std::string getInfo( std::string &noSpaceLine );
Directory &findPath( ServerConfig server, std::string path );