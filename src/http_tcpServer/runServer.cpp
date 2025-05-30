/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   runServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbourlot <hbourlot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 13:59:31 by hbourlot          #+#    #+#             */
/*   Updated: 2025/05/30 17:19:45 by hbourlot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_tcpServer/Http_tcpServer_linux.hpp"

int http::TcpServer::runServer() {

	int timeOut = 3 * 6 * 1000;

	if (startServer())
		return -1;
	try {
		startListen();
	} catch (const TcpServerException &e) {
		std::cerr << "Error while starting to listen => " << e.what()
				  << std::endl;
		close(m_serverSocket);
		return -1;
	}

	std::vector<pollfd> fds;
	struct pollfd listen_fd;
	listen_fd.fd = m_serverSocket;
	listen_fd.events = POLLIN; // any readable data available
	listen_fd.revents = 0;
	fds.push_back(listen_fd);

	runLoop(fds, timeOut);
	shutDownServer(fds);
	return 0;
}
