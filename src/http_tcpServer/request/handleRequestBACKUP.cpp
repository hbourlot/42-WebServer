#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <cstdio>
#include <fstream>
#include <netinet/in.h>
#include <ostream>
#include <sys/poll.h>
#include <vector>

static bool validateRequestMethod(const httpRequest &request, const Location &location) {

	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
		return false;

	for (size_t i = 0; i < location.methods.size(); ++i) {
		if (request.method == location.methods[i])
			return true;
	}
	return false;
}

static const Location *getMatchLocation(const std::string &path, const std::vector<Location> &locations) {

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i) {

		const std::string &locPath = locations[i].path;

		if (path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > matchLength) {
			matchedLocation = &locations[i];
			matchLength = locPath.size();
		}
	}
	return (matchedLocation);
}

namespace http {

	bool TcpServer::handleRequest(pollfd &socket, std::vector<pollfd> &fds, sockaddr_in &clientAddress) {

		const Location *matchedLocationPtr = getMatchLocation(_request.path, _serverInfo.locations);

		if (!matchedLocationPtr) {
			setFileResponse(HTTP_NOT_FOUND, _serverInfo.errorPage[404]);
			return false;
		}

		const Location &matchedLocation = *matchedLocationPtr;

		if (!matchedLocation.redirection.empty()) {
			setRedirect(HTTP_MOVED, matchedLocation.redirection);
			return (true);
		}

		if (!validateRequestMethod(_request, matchedLocation)) {
			setFileResponse(HTTP_FORBID_METHOD, DFL_405);
			return (false);
		}

		// * Handler CGI
		if (parseCgi(matchedLocation, clientAddress, _request, socket, fds)) {
			std::cout << "Entered here" << std::endl;
			// SocketFD fd = _cgi.back().getPollFd();
			// _cgiFdMap[socket.fd] = _cgi[0];

			// _cgi[0]->readCgiOutput();
			// std::cout << _cgi[0]->getOutputContent() << std::endl;

			// _test->readCgiOutput();
			// std::cout << _test->getOutputContent() << std::endl;

			// exit(0);
			// std::cout << "O FD => " << socket.fd << std::endl;
			return true;
		}

		// Set event POLLOUT only if it's not CGI
		socket.events |= POLLOUT;

		if (_request.method == "GET")
			return (handleGetRequest(matchedLocation, clientAddress));
		else if (_request.method == "POST")
			return (handlePostRequest(matchedLocation));
		else if (_request.method == "DELETE")
			return (handleDeleteRequest(matchedLocation));
		return (true);
	}

} // namespace http