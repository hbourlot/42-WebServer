#pragma once

#include "Config/Configs.hpp"

class Client;

namespace http {
	namespace RequestParser {
		bool parseRequestBuffer(Client& client, const ServerConfig& serverInfo);
	}
} // namespace http
