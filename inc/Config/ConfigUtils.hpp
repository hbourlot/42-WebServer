#pragma once
#include "Configs.hpp"
#include <string>

struct Server;
struct Location;

std::string removeSpace(std::string& line);
std::string	getInfo(std::string& noSpaceLine);
Location&   findPath(Server server, std::string path);