#include "Configs.hpp"

struct Configs;
struct ServerConfig;
struct Location;

struct SetLocation {
	static bool setLocationConfig( std::ifstream &confFd, std::string line, ServerConfig &server );
	static void setDefaultLocation( ServerConfig& server, Location& location );
	static void getMethods( std::string noSpaceLine,
                 std::vector< std::string > &methods );
};