/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joralves <joralves@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2025/10/26 01:28:49 by joralves         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "httpTcpServer/HttpStatus.hpp"
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

enum PARSE_STATUS { // ! PROB WILL BE REMOVED
	PARSE_INCOMPLETE,
	PARSE_TOO_LARGE,
	PARSE_OK,
};

enum REQUEST_STATUS {

	CLIENT_NOT_FOUND = 1, // ! if there's no client. what to do?? CGI
	MAIN_FUNCTIONALITY_PROBLEMS,
	REQUEST_EMPTY,
	REQUEST_READ_SUCCESSFULLY,
	PARSE_OK,
	PARSE_INCOMPLETE,
	PARSE_TOO_LARGE,
	URL_NOT_FOUND,
	URL_REDIRECT,

};

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

struct httpResponse {
	std::string statusCode;
	std::string statusMsg;
	std::string body;
	std::map< std::string, std::string > headers;

	void setDefaultHeaders();
	void setDefaultHeaders( httpRequest request );
	void addToHeader( std::string key, std::string value );
	std::string buildResponseString( const httpRequest &request );
};