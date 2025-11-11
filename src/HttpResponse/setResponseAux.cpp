#include "httpTcpServer/HttpTcpServerLinux.hpp"

// std::string readFileContent(const std::string &filePath)
// {
// 	std::ifstream file(filePath.c_str());
// 	if (!file.is_open())
// 		return "";

// 	std::ostringstream buffer;
// 	buffer << file.rdbuf();
// 	file.close();
// 	return buffer.str();
// }

//! De momento ponerlo aqui, no encaja del todo en otros archivos

bool httpRequest::shouldCloseConnection()
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Connection");

	if (it != headers.end())
	{
		std::string val = it->second;

		for (std::string::size_type i = 0; i < val.size(); ++i)
			val[i] = std::tolower(val[i]);

		return (val == "close");
	}
	else
	{
		if (serverProtocol == "HTTP/1.1")
			return (false);
		else
			return (true);
	}
}