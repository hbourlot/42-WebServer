#include "Session/SessionManager.hpp"
#include "Client/Client.hpp"
#include "httpTcpServer/Response.hpp"

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

void SessionManager::applyAuthFromResponse(Client &client, http::Response &response) {
    const std::map<std::string, std::string> &headers = response.getHeaders();
    std::map<std::string, std::string>::const_iterator itAuth = headers.find("X-Authenticated-User");

    if (itAuth == headers.end() || itAuth->second.empty())
        return;

    const std::string username = itAuth->second;

    const std::string previousId = client.getSessionID();
    Session &authSession = getSession(previousId);

    authSession.setSessionData("username", username);
    authSession.setSessionData("authenticated", "true");

    client.setSessionID(authSession.getSessionId());
    response.addToHeader("Set-Cookie", "sessionId=" + authSession.getSessionId() + "; Path=/; HttpOnly");
}
