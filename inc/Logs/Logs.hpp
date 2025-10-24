#pragma once
// #include "httpTcpServer/HttpUtils.hpp"
#include "utils.hpp"
#include <iostream>

#define NC "\033[0m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

enum Level
{
	INFO,
	WARN,
	ERROR
};
class Logs
{
  public:
	static void log(Level lvl, const std::string &msg);

  private:
	static void info(const std::string &msg);
	static void warning(const std::string &msg);
	static void error(const std::string &msg);
};
