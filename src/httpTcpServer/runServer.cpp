/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   runServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joralves <joralves@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 13:59:31 by hbourlot          #+#    #+#             */
/*   Updated: 2025/10/24 18:30:13 by joralves         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "httpTcpServer/HttpTcpServerLinux.hpp"

int http::TcpServer::runServer()
{

	int timeOut = 3 * 60 * 1000;

	if (startServer())
		return -1;
	try
	{
		startListen();
	}
	catch (const TcpServerException &e)
	{
		std::cerr << "Error while starting to listen => " << e.what() << std::endl;
		close(_serverSocket);
		return -1;
	}

	pollfd listen_fd;
	listen_fd.fd = _serverSocket;
	listen_fd.events = POLLIN; // any readable data available
	listen_fd.revents = 0;
	_fds.push_back(listen_fd);

	runLoop(timeOut);
	shutDownServer();
	return 0;
}
