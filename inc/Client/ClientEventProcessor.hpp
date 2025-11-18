#pragma once

#include "httpTcpServer/HttpTcpServerLinux.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>

namespace http {

	class ClientEventProcessor {
	  public:
		friend class Router;

		ClientEventProcessor( TcpServer &server );

		~ClientEventProcessor();

		void processRead( pollfd &pfd, Client *client );

		void processWrite( pollfd &pfd, Client *client, int index );

		void processCgiEvents( int fd, int index );

		void processClientEvents( int index );

		void registerCgi( http::Cgi *cgi );
		void cleanupCgi( http::Cgi *cgi );
		void processCgiOutput( http::Cgi *cgi, pollfd &pfd );

		/// @brief Checks if a CGI process completed with exit status 0.
		/// @param cgi Pointer to the Cgi object to check.
		/// @return true if CGI finished with status > 0, false otherwise.
		bool hasCgiFinished( Cgi *cgi ) const;

		/// @brief Checks if a CGI process completed successfully with exit status 0.
		/// @param cgi Pointer to the Cgi object to check.
		/// @return true if CGI exited normally with status 0, false otherwise.
		/// @note IMPORTANT: Only call this function if hasCgiFinished() returned true.
		///       Calling this before the process has finished will result in undefined behavior
		///       as the status will not be properly set by waitpid().
		bool hasCgiSuccessfullyFinished( Cgi *cgi ) const;

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

		/// @brief Validates and processes the route for the client's HTTP request
		///
		/// This function performs route validation for the given client, checking if the
		/// requested route is valid, accessible, and properly configured according to
		/// the server's routing rules and permissions.
		///
		/// @param client Reference to the Client object containing the HTTP request to validate
		/// @return true if the route validation succeeds, false otherwise
		bool handleRouteValidation( Client &client );
	};
} // namespace http