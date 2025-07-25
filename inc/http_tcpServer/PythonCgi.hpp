#pragma once

#include "Client/Client.hpp"
#include "Config/Configs.hpp"
#include "HttpStructs.hpp"
#include "ICgi.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/poll.h>
#include <vector>

namespace http {

	class PythonCgi : public ICgi {

	  public:
		PythonCgi(Client *client, ClientManager *manager, std::string &filePath);
		~PythonCgi();

		// Start CGI process
		// void execute(std::vector<pollfd> &fds);
		void execute(); // Interface

		// Check Cgi State
		bool isRunning();  // Interface
		bool isFinished(); // Interface
		bool hasError();   // Interface
		void setErrorStatus();
		void setRunningStatus();
		void setFinishedStatus();
		void setErrorStatusWLog(std::string msg);

		void sendResponse(); // Interface

		//
		httpResponse getCgiResponse() const;
		httpRequest getCgiRequest() const;
		std::string getFilePath() const;
		SocketFD getClientFd() const;
		CgiFd getCgiFd() const;
		int getPollFd() const;
		void saveCgiOutput();
		// Get the output content (header + body)
		std::string getOutput(); // Interface
		CGI_TYPE getCgiType() const;

		void registerPollFd(std::vector<pollfd> &fds) const; //! Might not be necessary
		void registerWithManager();
		int _bytesReceived;

	  private:
		CGI_TYPE _type;
		bool _status[3];
		Client *_client;
		ClientManager *_manager;

		httpRequest _request;
		std::string _output;
		httpResponse _response;
		Server _serverInfo;
		std::string _filePath;
		sockaddr_in _clientAddress;

		std::vector<char *> _envp;
		std::vector<std::string> _envStrings;

		// Pipe handling
		int _inputPipe[2];
		int _outputPipe[2];
		pid_t _pid;
		pid_t _processResult;

		void buildEnvStrings();
		void doDup();
		void handleChildProcess();
		void handleWait(pid_t &pid, bool *status);
	};

}; // namespace http
