/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joralves <joralves@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2025/06/27 10:53:03 by joralves         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "http_tcpServer/HttpStatus.hpp"
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

// #define HTTP_200 "200"
// #define HTTP_301 "301"
// #define HTTP_404 "404"
// #define HTTP_500 "500"

typedef int SocketFD;

struct Location;

// Only interface propose
enum headerKey
{
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

enum ParseStatus
{
	PARSE_INCOMPLETE,
	PARSE_TOO_LARGE,
	PARSE_OK,
};

struct httpRequest
{
	std::string method;
	std::string path;
	std::string serverProtocol; // For Cgi
	std::string pathInfo;       // For Cgi
	std::string pathTranslated;
	std::map<std::string, std::string> headers;
	std::string body;

	std::string queryString; // test
	std::string getType(std::string key, std::string value);
};

struct httpResponse
{
	std::string statusCode;
	std::string statusMsg;
	std::string body;
	std::map<std::string, std::string> headers;

	void setDefaultHeaders();
	void setDefaultHeaders(httpRequest &request);
	void addToHeader(std::string key, std::string value);
	// void setResponseError(std::string statusCode);
	// void setResponseError(const HttpStatusCode &status);
};

// ! Maybe its better encapsulate al information about the client and its state
// on a proper struct

struct ClientState
{
	httpRequest request;
	httpResponse response;
	std::string readBuffer;
	std::string writeBuffer;
	bool requestComplete;
	bool cgiInProgress;
	int clientFd;

	ClientState() : requestComplete(false), cgiInProgress(false), clientFd(-1)
	{
	}
};