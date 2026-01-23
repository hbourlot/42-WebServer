#pragma once

#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>

namespace http {

	/// @brief Event processor for handling client socket events and CGI execution in the HTTP server.
	///
	/// This class is responsible for processing all I/O events (POLLIN/POLLOUT) on client sockets
	/// and CGI pipes. It manages the complete lifecycle of client requests and CGI processes,
	/// including reading requests, parsing HTTP data, routing to handlers, executing CGI scripts,
	/// and sending responses back to clients.
	///
	/// Key responsibilities:
	/// - Process POLLIN/POLLOUT events for client sockets
	/// - Read and parse HTTP requests from clients
	/// - Route requests to appropriate handlers (static/CGI)
	/// - Manage CGI process lifecycle (launch, monitor, cleanup)
	/// - Build and send HTTP responses
	/// - Handle connection cleanup and error states
	class ClientEventProcessor {
	  public:
		friend class Router;

		ClientEventProcessor( TcpServer &server );

		~ClientEventProcessor();

		/// @brief Handles POLLIN events for client sockets - reads data and parses HTTP requests.
		/// @param pfd Poll file descriptor structure for the client socket.
		/// @param client Pointer to the Client object to read from.
		void processRead( pollfd &pfd, Client *client );

		/// @brief Handles POLLOUT events for client sockets - processes requests and sends responses.
		/// @param pfd Poll file descriptor structure for the client socket.
		/// @param client Pointer to the Client object to write to.
		/// @param index Index of the file descriptor in the server's poll array.
		void processWrite( pollfd &pfd, Client *client, int index );

		/// @brief Handles events for CGI pipe file descriptors - checks if CGI finished and processes output.
		/// @param fd File descriptor of the CGI pipe.
		/// @param index Index of the file descriptor in the server's poll array.
		void processCgiEvents( int fd, int index );

		/// @brief Main event dispatcher - routes events to appropriate handlers (client or CGI).
		/// @param index Index of the file descriptor in the server's poll array.
		void processClientEvents( int index );

		/// @brief Registers a CGI object in the server's map for tracking and event processing.
		/// @param cgi Pointer to the Cgi object to register (ownership transferred).
		void registerCgi( http::Cgi *cgi );
		
		/// @brief Cleans up CGI resources - kills process, closes pipes, removes from poll array and map.
		/// @param cgi Pointer to the Cgi object to cleanup (will be deleted).
		void cleanupCgi( http::Cgi *cgi );

		///@brief Reads CGI output from pipe and builds HTTP response based on exit status.
		///
		///Reads all available data from the CGI process's output pipe (non-blocking), handles errors, and constructs the HTTP response.
		///
		///@param cgi Pointer to the Cgi object to read output from.
		///@param pfd Poll file descriptor structure for the CGI pipe.
		///@note Should only be called after hasCgiFinished() returns true.
		///
		///@return int
		///  ->  1: More data may be available (hit MAX_READS_PER_EVENT, POLLIN still set)
		///  ->  0: Successfully read all data and built the CGI response
		///  -> -1: Read error occurred, error response built and cleanup performed
		int readCgiPipeAndBuildResponse( http::Cgi *cgi, pollfd &pfd );

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

		/// @brief Reads data from client socket into the read buffer (non-blocking).
		/// @param client Reference to the Client object to read from.
		/// @return true on successful read or would-block, false on error or connection closed.
		bool readFromSocket( Client &client );

		/// @brief Parses HTTP request data from client's read buffer and validates against server config.
		/// @param client Reference to the Client object with request data to parse.
		/// @param serverInfo Server configuration for validation (max body size, etc.).
		/// @return true if parsing succeeds, false on parse errors.
		bool parseRequestData( Client &client, const ServerConfig &serverInfo );

		/// @brief Sends HTTP response data from write buffer to client socket (non-blocking).
		/// @param pfd Poll file descriptor structure for the client socket.
		/// @param client Reference to the Client object to send response to.
		/// @return true if all data sent, false if more data remains or error occurred.
		bool sendResponse( pollfd &pfd, Client &client );

		/// @brief Builds HTTP response string if needed and sends it to the client.
		/// @param pfd Poll file descriptor structure for the client socket.
		/// @param client Reference to the Client object to handle response for.
		/// @return true if response sent successfully, false otherwise.
		bool handleResponse( pollfd &pfd, Client &client );

		/// @brief Closes client connection and removes it from the server's management.
		/// @param index Index of the client's file descriptor in the server's poll array.
		void closeConnection( size_t index );

		// Main request processing pipeline
		/// @brief Main request processing pipeline - validates request and executes routing.
		/// @param client Reference to the Client object to process request for.
		/// @return false if CGI started (async), true if response ready to send.
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
		bool handleRouteValidation( Client &client, VALIDATION_STATUS &validationStatus );
	};
} // namespace http