#pragma once
#include "Config/Configs.hpp"

struct ReadConfig {
	static bool setConfigs( char* conf, Configs& configs );
	static bool setServerConfig( std::ifstream& confFd, std::string& line, Configs& configs );
	static void setDefaultServer( ServerConfig& server );
	static void setServerLocations( Configs& Configs );
};