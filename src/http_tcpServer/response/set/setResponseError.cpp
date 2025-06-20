#include "http_tcpServer/Http_tcpServer_linux.hpp"

void httpResponse::setResponseError(std::string statusCode,
                                    std::string statusMsg) {
	this->statusCode = statusCode;
	this->statusMsg = statusMsg;

	addToHeader("Content-Type", "text/plain");

	this->body = statusMsg + " (" + statusCode + ")";
}