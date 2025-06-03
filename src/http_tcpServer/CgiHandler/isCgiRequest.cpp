/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isCgiRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbourlot <hbourlot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 20:48:16 by hbourlot          #+#    #+#             */
/*   Updated: 2025/06/03 21:00:45 by hbourlot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CgiHandler.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace HTTP {

	bool CgiHandler::isCgiRequest(httpRequest request) {
		std::string path = request.path;
		size_t qmark = path.find('?');
		if (qmark != std::string::npos)
			path = path.substr(0, qmark);

		size_t dot = path.rfind('.');
		std::string extension;
		if (dot != std::string::npos)
			extension = path.substr(dot);
		else
			extension = "";

		if (isValidCgiExtension(extension))
			return true;

		return false;
	}
} // namespace HTTP