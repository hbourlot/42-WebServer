#include "Configs.hpp"
#include <sys/stat.h>

struct Configs;
struct ServerConfig;
struct Directory;

struct SetLocation {
	static bool setLocationConfig( std::ifstream& confFd, std::string line, ServerConfig& server );
	static void setDefaultLocation( ServerConfig& server, Directory& location );
	static void getMethods( std::string noSpaceLine, std::vector< std::string >& methods );
};