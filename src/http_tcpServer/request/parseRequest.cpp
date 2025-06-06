#include "http_tcpServer/Http_tcpServer_linux.hpp"

static void parseRequestQueries(httpRequest &request)
{
	size_t idx;
	idx = request.path.find("?");
	if (idx == std::string::npos)
		return;
	request.queries = split(request.path.substr(idx + 1), '&');
	// for(int i =0; i < request.queries.size(); ++i)
	// 	std::cout << "request.queries[i] " << request.queries[i] << std::endl;
	request.path = request.path.substr(0, idx);
}

static void parseRequestHeaders(httpRequest &request,
                                std::istringstream &request_stream,
                                std::string &line)
{
	size_t idx;

	while (std::getline(request_stream, line))
	{
		if (line == "\r" || line == "")
			break;
		idx = line.find(":");
		if (idx != std::string::npos)
		{

			std::string key = line.substr(0, idx);
			std::string value = line.substr(idx + 1);
			key = ft_strtrim(key);
			value = ft_strtrim(value);
			request.headers[key] = value;
		}
	}
}

void parseRequest(httpRequest &request, const std::string &requestContent)
{
	std::istringstream request_stream(requestContent);
	std::string line;
	std::getline(request_stream, line);
	std::istringstream first_line(line);

	first_line >> request.method >> request.path >> request.protocol;
	parseRequestQueries(request);

	parseRequestHeaders(request, request_stream, line);

	std::string body;
	while (std::getline(request_stream, line))
		body += line + "\n";
	request.body = body;
}
