#pragma once

#include "HttpStructs.hpp"
#include <poll.h>
#include <vector>

// #ifndef BUFFER_SIZE
// #define BUFFER_SIZE 30720
// #endif

namespace http {

	class ICgi {
	  public:
		enum { RUNNING_IDX, FINISHED_IDX, ERROR_IDX };

		virtual ~ICgi() {};

		// Start CGI process
		virtual void execute(std::vector<pollfd> &fds) = 0;

		// Check Cgi State
		virtual bool isRunning() const = 0;
		virtual bool isFinished() const = 0;
		virtual bool hasError() const = 0;

		// Read output from the CGI process
		// virtual void readOutput() = 0;

		// Send the CGI response to the client (write to socket)
		virtual void sendResponse() = 0;

		// Get the output content (header + body)
		virtual std::string getOutput() const = 0;

		// Get the file path of the CGI script/file
		virtual std::string getFilePath() const = 0;

		// Get the client socket fd (for poll/select)
		virtual SocketFD getClientFd() const = 0;

		// Register the CGI`s pipe fd(s) with poll/select
		virtual void registerPollFd(std::vector<pollfd> &fds) const = 0;
	};

} // namespace http
