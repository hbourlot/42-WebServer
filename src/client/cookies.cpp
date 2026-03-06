#include "httpTcpServer/EventProcessor.hpp"

std::string getSessionIdFromCookies(const std::string& cookieHeader) {
	std::string sessionId;
	size_t pos = cookieHeader.find("sessionId=");
	if (pos == std::string::npos)
		return (sessionId);

	size_t start = cookieHeader.find("=", pos) + 1;

	size_t end = cookieHeader.find(';', start);
	if (end == std::string::npos)
		end = cookieHeader.length();

	sessionId = cookieHeader.substr(start, end - start);
	return (sessionId);
}

void ensureSessionId(Client& client) {

	std::string cookieHeader;
	if (client.getRequest().getHeaders().count("Cookie")) {
		cookieHeader = client.getRequest().getHeaders()["Cookie"];
	}

	std::string sessionId;
	if (!cookieHeader.empty()) {
		sessionId = getSessionIdFromCookies(cookieHeader);
	}
	if (!sessionId.empty()) {
		client.setSessionID(sessionId);
	}
}
