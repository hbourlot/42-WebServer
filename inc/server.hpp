/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mistery576 <mistery576@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 18:06:07 by miafonso          #+#    #+#             */
/*   Updated: 2025/05/10 21:46:36 by mistery576       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <netinet/in.h>

class Server{
	private:
        int         _serverSocket;
        sockaddr_in _serverAddress;
	
	public:
        
		Server();
		~Server();

        void bindServer();
};