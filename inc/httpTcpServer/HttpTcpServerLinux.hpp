#pragma once

#include "Cgi.hpp"
#include "Client/ClientManager.hpp"
#include "Config/CheckConf.hpp"
#include "Config/ReadConfig.hpp"
// #include "Client/ClientEventProcessor.hpp"
#include "HttpStatus.hpp"
#include "HttpStructs.hpp"
#include "Logs/Logs.hpp"
#include "Router.hpp"
#include "Upload/UploadManager.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <poll.h>
#include <set>
#include <sstream>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define DFL_404 "content/defaults/error_404.html"
#define DFL_405 "content/defaults/error_405.html"
#define DFL_500 "content/defaults/error_500.html"

#ifndef nullptr
#define nullptr NULL
#endif

// No flag for recv function
#ifndef MSG_NOFLAGS
#define MSG_NOFLAGS 0
#endif

class Cgi;

namespace http {

	class ClientEventProcessor;
	const int BUFFER_SIZE = 65536;
	const int MAX_READS_PER_EVENT = 3;

	class TcpServer {
	  public:
		friend class ClientEventProcessor;
		// Default Constructor
		TcpServer( ServerConfig server );
		// Default Destructor
		~TcpServer();

		// Main member
		int runServer();

		class TcpServerException : public std::runtime_error {
		  public:
			explicit TcpServerException( const std::string& message ) : std::runtime_error( message ) {
			}
		};
		std::vector< pollfd >& getVectorPollFds();

		std::vector< pollfd > _fds;

		ServerConfig& getServerInfo() {

			return _serverInfo;
		}

	  private:
		ServerConfig _serverInfo;
		SocketFD _serverSocket;
		ClientManager _clientManager;

		std::map< SocketFD, sockaddr_in > _socketAddressMap;
		unsigned int _socketAddress_len;

		std::map< int, http::Cgi* > _cgiByFd; // CGI output fd → Cgi instance

		int startServer();
		void runLoop( int timeOut );
		void shutDownServer();
		void startListen();
		void acceptConnection();
		void removeDeadConnections( ClientEventProcessor& processor, size_t& index );
		// void removeCgiDeadConnection( ClientEventProcessor &processor );
		void closeClientConnection( size_t index );
		void cleanupAllCgis();
	};

	std::string getLocationFieldAsString( const std::vector< Directory >& locations, const std::string& field );

} // namespace http
