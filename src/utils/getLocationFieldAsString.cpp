#include "Config/Configs.hpp"
#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sstream>
#include <string>
#include <vector>

std::string
http::getLocationFieldAsString(const std::vector<Location> &locations,
                               const std::string &field)
{
	std::ostringstream oss;
	bool first = true;
	for (size_t i = 0; i < locations.size(); ++i)
	{
		if (field == "path")
		{
			if (!first)
				oss << " ";
			oss << locations[i].path;
		}
		else if (field == "root")
		{
			if (!first)
				oss << " ";
			oss << locations[i].root;
		}
		else if (field == "index")
		{
			if (!first)
				oss << " ";
			oss << locations[i].index;
		}
		else if (field == "redirection")
		{
			if (!first)
				oss << " ";
			oss << locations[i].redirection;
		}
		else if (field == "uploadStore")
		{
			if (!first)
				oss << " ";
			oss << locations[i].uploadStore;
		}
		else if (field == "methods")
		{
			for (size_t j = 0; j < locations[i].methods.size(); ++j)
			{
				if (!first)
					oss << " ";
				oss << locations[i].methods[j];
				first = false;
			}
			continue;
		}
		else if (field == "cgi_extension")
		{
			for (size_t j = 0; j < locations[i].cgi_extension.size(); ++j)
			{
				if (!first)
					oss << " ";
				oss << locations[i].cgi_extension[j];
				first = false;
			}
			continue;
		}
		else if (field == "cgi_path")
		{
			for (size_t j = 0; j < locations[i].cgi_path.size(); ++j)
			{
				if (!first)
					oss << " ";
				oss << locations[i].cgi_path[j];
				first = false;
			}
			continue;
		}
		first = false;
	}
	return oss.str();
}
