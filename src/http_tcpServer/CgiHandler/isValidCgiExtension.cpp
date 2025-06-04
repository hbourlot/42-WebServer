#include "CGI/CgiHandler.hpp"

namespace http {

	const std::set<std::string> CgiHandler::validCgiExtensions =
		createValidCgiExtensions();

	bool CgiHandler::isValidCgiExtension(const std::string &ext) {
		return validCgiExtensions.count(ext) > 0;
	}
} // namespace http