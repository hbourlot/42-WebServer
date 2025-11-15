#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <iostream>

struct HttpStatusCode {
	const std::string code;
	const std::string message;
};

const HttpStatusCode HTTP_OK = { "200", "OK" };
const HttpStatusCode HTTP_PARTIAL_CONTENT = { "206", "Partial Content" };
const HttpStatusCode HTTP_MOVED = { "301", "Moved Permanently" };
const HttpStatusCode HTTP_BAD_REQ = { "400", "Bad Request" };
const HttpStatusCode HTTP_UPLOAD_FORBID = { "403", "Upload Not Allowed" };
const HttpStatusCode HTTP_NOT_FOUND = { "404", "Not Found" };
const HttpStatusCode HTTP_FORBID_METHOD = { "405", "Method Not Allowed" };
const HttpStatusCode HTTP_PAYLOAD = { "413", "Payload Too Large" };
const HttpStatusCode HTTP_RANGE_NOT_SATISFIABLE = { "416", "Range Not Satisfiable" };
const HttpStatusCode HTTP_UNSUPPORTED_MEDIA = { "415", "Unsupported Media Type" };
const HttpStatusCode HTTP_SERVER_ERR = { "500", "Internal Server Error" };

#endif
