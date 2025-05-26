#include "Configs.hpp"

struct Configs;
struct Server;
struct Location;

struct SetLocation{
    static bool	setLocationConfig(std::ifstream& confFd, std::string line, Server& server);
	static void	setDefaultLocation(Location& location);
};