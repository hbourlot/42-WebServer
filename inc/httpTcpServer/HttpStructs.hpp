/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joralves <joralves@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2026/01/22 15:06:33 by joralves         ###   ########.fr       */
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

struct MatchResult {
	const Location *location;
	const File *file;
};

struct RouteContext {
	// 🔹 Permissions
	std::vector< std::string > methods;

	// 🔹 resourse
	std::string path;
	std::string root;
	std::string index;

	// 🔹 CGI
	bool isCgi;
	std::string cgi_pass;

	// 🔹 Redirection
	bool isRedirect;
	std::string redirection;

	// 🔹 Request limits
	size_t max_body_size;

	// 🔹 Directory handling
	bool autoIndex;

	// 🔹 Upload
	bool uploadEnable;
	std::string uploadStore;
};

enum VALIDATION_STATUS {

	VALID_OK = 1,
	VALID_METHOD_NOT_ALLOWED,
	VALID_NOT_FOUND,
	VALID_FORBIDDEN,
	VALID_REDIRECT_REQUIRED,
	VALID_IS_CGI,

};

namespace http {

	struct Request {
		std::string method;
		std::string path;
		std::string serverProtocol;
		std::string pathInfo; // For Cgi
		std::string pathTranslated;
		std::map< std::string, std::string > headers;
		std::string body;
		std::string GetFileName();
		std::string rawRequestBuffer;
		std::string queryString;
		MatchResult matchResult;
		// const Location *urlMatchedLocation; // ! Must Initialize as NULL;
	};

} // namespace http

// For RouteContext
RouteContext makeContext( const MatchResult &match, const ServerConfig &server,http::Request &request, VALIDATION_STATUS status );
std::string getFilePath( const std::string &path, const RouteContext &ctx );
