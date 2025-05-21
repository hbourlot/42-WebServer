#pragma once

#include "http_tcpServer_linux.hpp"
#include <exception>
#include <stdexcept>
#include <unistd.h>

namespace http {

	class TcpServerException : public std::runtime_error {

		public:
			explicit TcpServerException(const std::string &message)
				: std::runtime_error(message) {
			}
	};

} // namespace http