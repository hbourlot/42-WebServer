#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	bool TcpServer::handlePostRequest(const Location *location)
	{
		if (!location->cgi_path.empty())
		{
			std::cout << "HERE CGI POST" << std::endl;
		}

		if (request.path == "/login")
		{
			httpResponse result = validateForm(request);

			if (!result.body.empty())
			{
				setHtmlResponse(result.statusCode, result.statusMsg,
				                result.body);
			}
			else
				setResponseError(result.statusCode, result.statusMsg);
		}
		else if (location->uploadEnable)
		{
			std::cout << request.headers["Content-Type"] << std::endl;
			// if
			parseMultipart(location);
		}
		else if (!location->uploadEnable)
		{
			setResponseError("403", "Upload Not Allowed");
			return (false);
		}
		else
		{
			setHtmlResponse("404", "Not Found", infos.errorPage[404]);
			return (false);
		}
		return true;
	}

} // namespace http
