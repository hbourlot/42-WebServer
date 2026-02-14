#pragma once

#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include "utils.hpp"
// #include "httpTcpServer/HttpStructs.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>

namespace http {

	class ClientEventProcessor {
		public:
			friend class Router;

			class ClientEventProcessorException : public std::runtime_error {
				public:
					explicit ClientEventProcessorException( const std::string &msg ) : std::runtime_error( msg ) {};
			};

			ClientEventProcessor( std::vector< pollfd > &allServerFds, std::vector< TcpServer * > servers );

			~ClientEventProcessor();

			void run();

			void acceptConnections();

			bool removeDeadConnections( size_t &index );

			void shutDownProcessor();

			void processRead( pollfd &pfd, Client *client, Cgi *cgi );

			void processWrite( pollfd &pfd, Client *client, int index );

			void processClientEvents( int index );

			void registerCgi( http::Cgi *cgi );

			void cleanupCgi( http::Cgi *cgi );

			bool hasCgiFinished( Cgi *cgi ) const;

			bool hasCgiSuccessfullyFinished( Cgi *cgi ) const;

		private:
			std::vector< pollfd > &_allSockets;
			std::vector< struct sockaddr_in > _socketAddress;
			size_t _serverSocketSize;
			std::vector< TcpServer * > _servers;
			ClientManager _clientManager;
			std::map< SocketFD, Cgi * > _cgi_by_fd;

			size_t _clientIndex;

			void checkIdleConnections( size_t index );

			void closeClientConnection( size_t index );

			bool readFromSocket( SocketFD fd, std::string &readBuffer, IN_OUT_STATE &state );

			bool parseRequestData( Client &client, const ServerConfig &serverInfo );

			bool sendResponse( pollfd &pfd, Client &client );

			bool handleResponse( pollfd &pfd, Client &client );


			bool processRequest( Client &client );

			bool buildErrorResponse( Client &client, IN_OUT_STATE state );

			bool handleRouteValidation( Client &client, VALIDATION_STATUS &validationStatus );
	};
} // namespace http