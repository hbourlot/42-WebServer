#pragma once

#include "Session.hpp"
#include <map>
#include <string>

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    Session & getSession(std::string const & sessionId);
    void deleteSession(std::string const & sessionId);
    Session & createSession();

private:
    std::map<std::string, Session> _sessions;
};