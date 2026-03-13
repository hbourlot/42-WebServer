#pragma once

#include "Session.hpp"
#include <map>
#include <string>

class Client;
namespace http {
    class Response;
}

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    Session & getSession(std::string const & sessionId);
    void deleteSession(std::string const & sessionId);
    Session & createSession();
	void applyAuthFromResponse(Client &client, http::Response &response);

private:
    std::map<std::string, Session> _sessions;
};