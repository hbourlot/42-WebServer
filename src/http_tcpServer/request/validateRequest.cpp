#include "CGI/CgiHandler.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <vector>

namespace http
{

	const Location *getMatchLocation(const std::string &path,
	                                 const std::vector<Location> &locations)
	{

		const Location *matchedLocation = NULL;
		size_t matchLength = 0;

		for (size_t i = 0; i < locations.size(); ++i)
		{

			const std::string &locPath = locations[i].path;

			if (path.compare(0, locPath.size(), locPath) == 0 &&
			    locPath.size() > matchLength)
			{
				matchedLocation = &locations[i];
				matchLength = locPath.size();
			}
		}
		return (matchedLocation);
	}

	static bool validateRequestMethod(httpRequest request,
	                                  const Location *location)
	{

		if (request.method != "GET" && request.method != "POST" &&
		    request.method != "DELETE")
			return false;

		for (size_t i = 0; i < location->methods.size(); ++i)
		{
			if (request.method == location->methods[i])
				return true;
		}
		return false;
	}

	// static bool handleCgiMethod(httpRequest request) {
	// }

	bool TcpServer::validateRequest()
	{

		const Location *matchedLocation =
		    getMatchLocation(request.path, infos.locations);

		if (!matchedLocation)
		{
			setHtmlResponse("404", "Not Found", infos.errorPage[404]);
			return false;
		}
		//! Treat before the /redirect-me
		if (validateRequestMethod(request, matchedLocation) == false)
		{
			setHtmlResponse("405", "Method Not Allowed", DFL_405);
			return (false);
		}

		// !!! HERE
		// if (CgiHandler::isCgiRequest(request)) {
		// 	int i = 0;
		// 	std::string ext =
		// 		getLocationFieldAsString(infos.locations, "cgi_extension");
		// 	std::vector<std::string> ext_splitted = split(ext, ' ');

		// 	// Maybe verify all cgi_extensions instead of just once by once
		// 	while (CgiHandler::isValidCgiExtension(ext_splitted[i]))
		// 		i++;
		// 	if (i == ext.size()){
		// 		// handleCgiRequest
		// 	}
		// }

		// std::cout << "HELLO RIGHTTTTT\n";
		// std::cout << matchedLocation->path << "                         "<<
		// matchedLocation->cgi_path[0] <<std::endl;
		if (request.method == "GET")
			return (handleGetRequest(matchedLocation));
		else if (request.method == "POST")
			return (handlePostRequest(matchedLocation));
		else if (request.method == "DELETE")
			return(handleDeleteRequest(matchedLocation));
		return (true);
	}

} // namespace http