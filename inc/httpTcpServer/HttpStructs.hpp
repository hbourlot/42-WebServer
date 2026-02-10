/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joralves <joralves@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:32:48 by hbourlot          #+#    #+#             */
/*   Updated: 2026/02/09 18:41:55 by joralves         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "httpTcpServer/HttpStatus.hpp"
#include "httpTcpServer/Request.hpp"
#include "httpTcpServer/Response.hpp"
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

typedef int SocketFD;

struct Directory;

struct MatchResult;

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

enum VALIDATION_STATUS {

	VALID_OK = 1,
	VALID_METHOD_NOT_ALLOWED,
	VALID_NOT_FOUND,
	VALID_FORBIDDEN,
	VALID_REDIRECT_REQUIRED,
	VALID_IS_CGI,

};
