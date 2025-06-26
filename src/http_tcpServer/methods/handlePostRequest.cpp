#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	bool TcpServer::handlePostRequest(const Location &location)
	{
		if (!location.cgi_path.empty())
		{
			std::cout << "HERE CGI POST" << std::endl;
		}

		// if (request.path == "/login")
		// {
		// 	httpResponse result = validateForm(request);

		// 	if (!result.body.empty())
		// 	{
		// 		// setFileResponse(result.statusCode, result.statusMsg,
		// 		// result.body);
		// 	}
		// 	// else
		// 	// setResponseError(result.statusCode, result.statusMsg);
		// }
		else if (location.uploadEnable)
		{
			std::cout << "_request.headers " << _request.headers["Content-Type"]
			          << std::endl;
			// if(_request.headers["Content-Type"] == "")
			parseMultipart(location);
		}
		else if (!location.uploadEnable)
		{
			setResponseError(HTTP_UPLOAD_FORBID);
			return (false);
		}
		else
		{
			setFileResponse(HTTP_NOT_FOUND, _infos.errorPage[404], true);
			return (false);
		}
		return true;
	}

} // namespace http
