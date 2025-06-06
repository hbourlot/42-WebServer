// #include "http_tcpServer/http_tcpServerException_linux.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"

httpResponse makeRedirectResponse(const std::string &location)
{
	httpResponse res;
	res.statusCode = "302";
	res.statusMsg = "Found";
	res.body = "";
	res.headers["Location"] = location;
	return res;
}

httpResponse makeHttpResponse(const std::string statusCode,
                              const std::string statusMsg,
                              const std::string body = "")
{
	httpResponse result;
	result.statusCode = statusCode;
	result.statusMsg = statusMsg;
	result.body = body;
	return (result);
}

static std::map<std::string, std::string> parseForm(const std::string &body)
{
	std::map<std::string, std::string> form;
	std::istringstream stream(body);
	std::string pair;
	while (std::getline(stream, pair, '&'))
	{
		ssize_t equal = pair.find('=');
		if (equal != std::string::npos)
		{
			std::string key = pair.substr(0, equal);
			std::string value = pair.substr(equal + 1);
			key = ft_strtrim(key);
			value = ft_strtrim(value);
			form[key] = value;
		}
	}
	return (form);
}

static httpResponse formUrlEncoded(httpRequest request)
{
	httpResponse result;
	std::map<std::string, std::string> form = parseForm(request.body);
	if (form["username"] == "admin" && form["password"] == "1234")
		return makeRedirectResponse("/dashboard.html");
	else
		return (makeHttpResponse("401", "Unauthorized"));
	return (result);
}

httpResponse validateForm(httpRequest request)
{
	httpResponse result;
	std::cout << request.headers["Content-Type"] << std::endl;
	if (request.headers["Content-Type"] == "application/x-www-form-urlencoded")
		result = formUrlEncoded(request);
	else if (request.headers["Content-Type"] == "application/json")
	{
		std::cout << "json form not valids for now" << std::endl;
		result = makeHttpResponse("401", "Unauthorized", "");
	}
	return (result);
}
