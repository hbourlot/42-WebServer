#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

struct HttpStatusCode
{
	const char *code;
	const char *message;
};

const HttpStatusCode HTTP_OK = {"200", "OK"};
const HttpStatusCode HTTP_MOVED = {"301", "Moved Permanently"};
const HttpStatusCode HTTP_BAD_REQ = {"400", "Bad Request"};
const HttpStatusCode HTTP_NOT_FOUND = {"404", "Not Found"};
const HttpStatusCode HTTP_FORBID_METHOD = {"405", "Method Not Allowed"};
const HttpStatusCode HTTP_PAYLOAD = {"413", "Payload Too Large"};
const HttpStatusCode HTTP_SERVER_ERR = {"500", "Internal Server Error"};

#endif
