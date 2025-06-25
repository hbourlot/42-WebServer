#include "http_tcpServer/Http_tcpServer_linux.hpp"

void httpResponse::setResponseError(const HttpStatusCode &status)
{
	this->statusCode = status.code;
	this->statusMsg = status.message;

	addToHeader("Content-Type", "text/plain");

	this->body = statusMsg + " (" + statusCode + ")";
	setDefaultHeaders();
}