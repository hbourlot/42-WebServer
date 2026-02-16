#pragma once

#include <string>
#include <map>
#include <ctime>

class Session {
public:
    Session();
    ~Session();

    std::string const & getSessionId() const;
    void setSessionData(std::string const & key, std::string const & value);
    std::string getSessionData(std::string const & key);
    bool isExpired() const;
    void refresh();

private:
    std::string _sessionId;
    std::map<std::string, std::string> _sessionData;
    time_t _expiryTime;

    std::string generateSessionId();
};