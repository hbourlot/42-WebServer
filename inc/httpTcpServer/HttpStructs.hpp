/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hugobourlot <hugobourlot@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2025/12/27 15:16:48 by hugobourlot      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "httpTcpServer/HttpStatus.hpp"
#include "httpTcpServer/Response.hpp"
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

typedef int SocketFD;

struct Location;

// Only interface propose
enum headerKey {
	Accept,
	AcceptEncoding,
	AcceptLanguage,
	Connection,
	ContentType,
	Cookie,
	Host,
	Priority,
	SecFetchDest,
	SecFetchMode,
	SecFetchSite,
	UpgradeInsecureRequests,
	UserAgent,
};

namespace http {

	struct Request {
		std::string method;
		std::string path;
		std::string serverProtocol;
		std::string pathInfo;       // For Cgi
		std::string pathTranslated;
		std::map< std::string, std::string > headers;
		std::string body;
		std::string GetFileName();
		std::string rawRequestBuffer;
		std::string queryString;
		const Location *urlMatchedLocation; // ! Must Initialize as NULL;
	};

} // namespace http