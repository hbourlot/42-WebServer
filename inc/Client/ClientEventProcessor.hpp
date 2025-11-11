#pragma once

#include "httpTcpServer/HttpTcpServerLinux.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>

namespace http {

	// const int BUFFER_SIZE = 30720;

	class ClientEventProcessor {
	  public:
		ClientEventProcessor( TcpServer &server );

		~ClientEventProcessor();

		void processRead( pollfd &pfd, Client &client );

		void processWrite( pollfd &pfd, Client &client );

		void processClientEvents();

	  private:
		TcpServer &_server;
		size_t _clientIndex;

		bool readFromSocket( Client &client );
		bool parseRequestData( Client &client, const ServerConfig &serverInfo );
		bool sendResponse( pollfd &pfd, Client &client );

		void closeConnection( size_t index );

		// Main request processing pipeline
		bool processRequest( Client &client );

		// Error handling
		bool buildErrorResponse( Client &client, CLIENT_STATE state );

		// Successful request handling
		bool handleSuccessfulRequest( Client &client );

		// Request validation (URL, method, permissions, redirects)
		bool handleRouteValidation( Client &client );

		void executeRequest( Client &client );
	};
} // namespace http