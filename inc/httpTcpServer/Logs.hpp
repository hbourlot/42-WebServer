#pragma once
#include "HttpStructs.hpp"

namespace {
	void log_prev( const std::string &message ) {
		std::cout << message << std::endl;
	}

	void exitWithError( const std::string &errorMessage ) {
		log_prev( "ERROR: " + errorMessage );
		exit( 1 ); // Use exit(1) to indicate an error
	}

	void logDebugger( const std::string &message ) {
		std::cout << "Debugger => " << message << std::endl;
	}

} // namespace
