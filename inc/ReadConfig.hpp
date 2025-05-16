#pragma once
#include "Configs.hpp"

struct ReadConfig{
    static bool setConfigs(char* conf, Configs& configs);
	static bool setServerConfig(std::ifstream& confFd, std::string& line, Configs& configs);
};