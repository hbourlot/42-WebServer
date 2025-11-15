/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hugobourlot <hugobourlot@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2025/11/15 12:52:42 by hugobourlot      ###   ########.fr       */
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

// enum READ_STATUS {
// 	READ_SUCCESS,
// 	READ_INCOMPLETE,
// 	READ_ERROR,
// 	READ_CLIENT_CLOSED,
// 	READ_CLIENT_NOT_FOUND,
// 	READ_EMPTY
// };

// enum PARSE_STATUS {
// 	PARSE_INCOMPLETE,
// 	PARSE_TOO_LARGE,
// 	PARSE_OK,
// 	PARSE_ERROR,
// };

struct httpRequest {
	std::string method;
	std::string path;
	std::string serverProtocol; // For Cgi
	std::string pathInfo;       // For Cgi
	std::string pathTranslated;
	std::map< std::string, std::string > headers;
	std::string body;
	bool shouldCloseConnection();
	std::string rawRequestBuffer;
	std::string queryString;
	const Location *urlMatchedLocation; // ! Must Initialize as NULL;
};
