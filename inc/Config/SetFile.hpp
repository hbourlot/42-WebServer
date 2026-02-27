#include "Configs.hpp"

struct Configs;
struct ServerConfig;
struct File;

struct SetFile {
	static bool setFileConfig( std::ifstream &confFd, std::string line, ServerConfig &server );
	static void setDefaultFile( File &location, ServerConfig &server);
};