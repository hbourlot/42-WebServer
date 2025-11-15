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
		friend class Router;

		ClientEventProcessor( TcpServer &server );

		~ClientEventProcessor();

		void processRead( pollfd &pfd, Client &client );

		void processWrite( pollfd &pfd, Client &client );

		void processClientEvents();

		void registerCgiPipes( const int inputPipe[ 2 ], const int outputPipe[ 2 ], pid_t pid );
		void closeCgiPipes( pid_t pid );
		void registerCgiForClient( Client &client, int cgiOutputFd );
		void cleanupCgiForClient( Client &client );
		void processCgiOutput(Client &client, pollfd &pfd);

	  private:
		TcpServer &_server;
		size_t _clientIndex;

		bool readFromSocket( Client &client );
		bool parseRequestData( Client &client, const ServerConfig &serverInfo );
		bool sendResponse( pollfd &pfd, Client &client );
		bool handleResponse( pollfd &pfd, Client &client );

		void closeConnection( size_t index );

		// Main request processing pipeline
		bool processRequest( Client &client );

		/// @brief Builds an HTTP error response based on client state
		/// @param client Client to send error response to
		/// @param state Error state (READ_ERROR, READ_EMPTY, PARSE_TOO_LARGE, etc.)
		/// @return true after building error response
		bool buildErrorResponse( Client &client, CLIENT_STATE state );

		/// @brief Handles successful request processing and response generation
		/// @param client Client with successfully parsed and validated request
		/// @return true on successful handling, false on error
		bool handleSuccessfulRequest( Client &client );

		/// @brief Validates and processes the route for the client's HTTP request
		/// 
		/// This function performs route validation for the given client, checking if the
		/// requested route is valid, accessible, and properly configured according to
		/// the server's routing rules and permissions.
		/// 
		/// @param client Reference to the Client object containing the HTTP request to validate
		/// @return true if the route validation succeeds, false otherwise
		bool handleRouteValidation( Client &client );

		void executeRequest( Client &client );
	};
} // namespace http