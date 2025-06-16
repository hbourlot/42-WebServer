#include "CGI/CgiHandler.hpp"
#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <fstream>
#include <ostream>
#include <vector>

namespace http
{
	static const Location *
	getMatchLocation(const std::string &path,
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

	static bool validateRequestMethod(const httpRequest &request,
	                                  const Location &location)
	{

		if (request.method != "GET" && request.method != "POST" &&
		    request.method != "DELETE")
			return false;

		for (size_t i = 0; i < location.methods.size(); ++i)
		{
			if (request.method == location.methods[i])
				return true;
		}
		return false;
	}

	bool TcpServer::validateRequest()
	{

		const Location *matchedLocationPtr =
		    getMatchLocation(request.path, infos.locations);

		if (!matchedLocationPtr)
		{
			setFileResponse("404", "Not Found", infos.errorPage[404]);
			return false;
		}

		const Location &matchedLocation = *matchedLocationPtr;

		if (!matchedLocation.redirection.empty())
		{
			//! Treat before the /redirect-me
			std::cout << "Handle redirection " << std::endl;
			//* redirection 301
		}

		if (!validateRequestMethod(request, matchedLocation))
		{
			setFileResponse("405", "Method Not Allowed", DFL_405);
			return (false);
		}

		if (request.method == "GET")
			return (handleGetRequest(matchedLocation));
		else if (request.method == "POST")
			return (handlePostRequest(matchedLocation));
		else if (request.method == "DELETE")
			return (handleDeleteRequest(matchedLocation));
		return (true);
	}

} // namespace http