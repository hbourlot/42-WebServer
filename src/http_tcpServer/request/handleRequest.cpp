#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <netinet/in.h>
#include <ostream>
#include <sys/poll.h>
#include <vector>

namespace http {
	// static const Location *
	// getMatchLocation(const std::string &path,
	//                  const std::vector<Location> &locations) {

	// 	const Location *matchedLocation = NULL;
	// 	size_t matchLength = 0;

	// 	for (size_t i = 0; i < locations.size(); ++i) {

	// 		const std::string &locPath = locations[i].path;

	// 		if (path.compare(0, locPath.size(), locPath) == 0 &&
	// 		    locPath.size() > matchLength) {
	// 			matchedLocation = &locations[i];
	// 			matchLength = locPath.size();
	// 		}
	// 	}
	// 	return (matchedLocation);
	// }

	// static bool validateRequestMethod(const httpRequest &request,
	//                                   const Location &location) {

	// 	if (request.method != "GET" && request.method != "POST" &&
	// 	    request.method != "DELETE")
	// 		return false;

	// 	for (size_t i = 0; i < location.methods.size(); ++i) {
	// 		if (request.method == location.methods[i])
	// 			return true;
	// 	}
	// 	return false;
	// }

	// bool TcpServer::handleRequest(pollfd &socket,
	//                               sockaddr_in &clientAddress) {

	// 	const Location *matchedLocationPtr =
	// 	    getMatchLocation(_request.path, _serverInfo.locations);

	// 	if (!matchedLocationPtr) {
	// 		setFileResponse(HTTP_NOT_FOUND, _serverInfo.errorPage[404]);
	// 		return false;
	// 	}

	// 	const Location &matchedLocation = *matchedLocationPtr;

	// 	if (!matchedLocation.redirection.empty()) {
	// 		setRedirect(HTTP_MOVED, matchedLocation.redirection);
	// 		return (true);
	// 	}

	// 	if (!validateRequestMethod(_request, matchedLocation)) {
	// 		setFileResponse(HTTP_FORBID_METHOD, DFL_405);
	// 		return (false);
	// 	}

	// 	// * Handler CGI
	// 	// std::string filePath = getFilePath(_request.path, matchedLocation);
	// 	// std::string prototypeFilePath = filePath.substr(1);
	// 	// if (parseCgi(matchedLocation, prototypeFilePath, clientAddress,
	// 	//              _request)) {
	// 	// 	_cgi[0].executeCgi(_fds);
	// 	// 	_cgi[0].markAsRunning();
	// 	// 	_cgiFdMap[_cgi[0].getPollFd()] = &_cgi[0];
	// 	// 	return true;
	// 	// }

	// 	// Set event POLLOUT only if it's not CGI
	// 	// socket.events |= POLLOUT;

	// 	if (_request.method == "GET")
	// 		return (handleGetRequest(matchedLocation, clientAddress));
	// 	else if (_request.method == "POST")
	// 		return (handlePostRequest(matchedLocation));
	// 	else if (_request.method == "DELETE")
	// 		return (handleDeleteRequest(matchedLocation));
	// 	return (true);
	// }

} // namespace http