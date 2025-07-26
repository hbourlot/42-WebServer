#pragma once

#include "../Client/ClientManager.hpp"
#include "HttpStructs.hpp"
#include <poll.h>
#include <vector>

namespace http {
	enum CGI_TYPE { PYTHON_CGI, PHP_CGI };

	typedef int CgiFd;

	class ICgi {

	  public:
		enum { RUNNING_IDX, FINISHED_IDX, ERROR_IDX };

		virtual ~ICgi() {};

		// Start CGI process
		// virtual void execute(std::vector<pollfd> &fds) = 0;
		virtual void execute() = 0;

		// Check Cgi State
		virtual bool isRunning() = 0;
		virtual bool isFinished() = 0;
		virtual bool hasError() = 0;

		// Read output from the CGI process
		// virtual void readOutput() = 0;

		// Send the CGI response to the client (write to socket)
		virtual void sendResponse() = 0;

		// Get the output content (header + body)
		virtual std::string getOutput() = 0;

		// Get the file path of the CGI script/file
		virtual std::string getFilePath() const = 0;

		virtual CGI_TYPE getCgiType() const = 0;
	};

} // namespace http
