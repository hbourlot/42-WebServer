#include "http_tcpServer/Http_tcpServer_linux.hpp"

namespace http
{

	void TcpServer::setFileResponse(std::string statusCode,
	                                std::string statusMsg,
	                                const std::string &filePath, bool isError)
	{

		std::string content = readFileContent(filePath);
		if (content.empty())
		{
			if (!isError)
				setFileResponse("404", "Not Found", infos.errorPage[404], true);
			else
			{
				response.setResponseError(statusCode, statusMsg);
				response.setDefaultHeaders(request);
				setResponse();
			}
			return;
		}

		response.statusCode = statusCode;
		response.statusMsg = statusMsg;
		response.body = content;
		response.addHeader("Content-Type", getContentType(filePath));
		response.setDefaultHeaders(request);
		setResponse();
	}
} // namespace http