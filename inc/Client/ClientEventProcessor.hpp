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

		void processRead( pollfd &pfd, Client &client ) {

			if ( !readFromSocket( client ) )
				return;
			if ( !parseRequestData( client, _server._serverInfo ) )
				return;
			pfd.events |= POLLOUT; // Set to POLL OUT
		}

		void processWrite( pollfd &pfd, Client &client ) {

			if ( !buildResponse( client ) )
				return;
			sendResponse( pfd, client );
		}

	  private:
		TcpServer &_server;
		size_t _clientIndex;

		bool readFromSocket( Client &client );
		bool parseRequestData( Client &client, const ServerConfig &serverInfo );
		bool sendResponse( pollfd &pfd, Client &client );

		void closeConnection( size_t index );

		bool buildResponse( Client &client );

		bool handleRouteValidation( Client &client );

		void routeClientRequest( Client &client );
	};
} // namespace http