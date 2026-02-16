#include "Session/SessionManager.hpp"

SessionManager::SessionManager() {}

SessionManager::~SessionManager() {}

Session & SessionManager::getSession(std::string const & sessionId) {
    if (_sessions.find(sessionId) != _sessions.end() && !_sessions[sessionId].isExpired()) {
        _sessions[sessionId].refresh();
        return _sessions[sessionId];
    }
    return createSession();
}

void SessionManager::deleteSession(std::string const & sessionId) {
    _sessions.erase(sessionId);
}

Session & SessionManager::createSession() {
    Session newSession;
    _sessions[newSession.getSessionId()] = newSession;
    return _sessions[newSession.getSessionId()];
}
