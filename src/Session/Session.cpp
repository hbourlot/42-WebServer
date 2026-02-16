#include "Session/Session.hpp"
#include <cstdlib>
#include <sstream>

Session::Session() {
    _sessionId = generateSessionId();
    refresh();
}

Session::~Session() {}

std::string const & Session::getSessionId() const {
    return _sessionId;
}

void Session::setSessionData(std::string const & key, std::string const & value) {
    _sessionData[key] = value;
}

std::string Session::getSessionData(std::string const & key) {
    if (_sessionData.find(key) != _sessionData.end()) {
        return _sessionData[key];
    }
    return "";
}

bool Session::isExpired() const {
    return time(0) > _expiryTime;
}

void Session::refresh() {
    _expiryTime = time(0) + 3600; // 1 hour expiry
}

std::string Session::generateSessionId() {
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << (rand() % 16);
    }
    return ss.str();
}
