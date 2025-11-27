#pragma once
// #include "httpTcpServer/HttpUtils.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sstream>

#define NC "\033[0m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

#ifndef DEBUG
#define DEBUG false
#endif

enum Level { LOGS_INFO, LOGS_WARN, LOGS_ERROR };
class Logs {
  public:
	static void log( Level lvl, const std::string &msg );
	static void logAcceptError( const sockaddr_in &socketAddress );

  private:
	static void info( const std::string &msg );
	static void warning( const std::string &msg );
	static void error( const std::string &msg );
};
