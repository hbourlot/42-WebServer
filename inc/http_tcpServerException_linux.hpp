#pragma once

#include "http_tcpServer_linux.hpp"
#include <exception>
#include <stdexcept>

namespace http {

	class TcpServerException : public std::runtime_error {

		public:
			explicit TcpServerException(const std::string &message)
				: std::runtime_error(message) {
			}
	};

} // namespace http