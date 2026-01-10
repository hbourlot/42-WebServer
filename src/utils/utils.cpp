#include <sstream>

#include "httpTcpServer/HttpTcpServerLinux.hpp"

bool isDirectory(const std::string &filePath)
{
	struct stat s;
	if (stat(filePath.c_str(), &s) != 0)
		return (false);
	return (S_ISDIR(s.st_mode));
}

std::string getFilePath(const std::string &path, const Location &location)
{
	// std::cout << "path: " << path << std::endl;
	std::string relativePath = path.substr(location.path.length());
	std::string filePath = joinPath(location.root, relativePath);

	// std::cout << "filePath: " << filePath << std::endl;
	return (filePath);
}

std::string joinPath(const std::string &base, const std::string &sub)
{
	if (!base.empty() && (base[base.length() - 1] == '/' || sub[0] == '/'))
		return (base + sub);
	return (base + "/" + sub);
}

std::string getContentType(const std::string &path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream"; // generic Binary

	std::string ext = path.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "txt")
		return "text/plain";
	if (ext == "pdf")
		return "application/pdf";
	return "application/octet-stream";
}

std::string ft_strtrim(const std::string &str)
{
	unsigned int start = 0;
	unsigned int end = str.length();

	while (start < str.length() && std::isspace(str[start]))
		start++;
	while (end > start && std::isspace(str[end - 1]))
		end--;
	return (str.substr(start, end - start));
}

std::string dateString()
{
	time_t timestamp;
	time(&timestamp);
	std::string date = ctime(&timestamp);
	if (!date.empty() && date[date.length() - 1] == '\n')
		date.erase(date.length() - 1);
	return (date);
}

// Returns false, if the state is the same. Otherwise we return false
bool containBrackets(std::string &line, bool &state, std::string extraStringToFind)
{
	// If we find the new string
	if (!extraStringToFind.empty())
		return (line.find(extraStringToFind) != std::string::npos);

	if (line.find('#') != std::string::npos) // Check if it is a comment
		return true;

	int openCount = 0;
	int closeCount = 0;
	for (size_t i = 0; i < line.size(); ++i)
	{
		if (line[i] == '{')
			openCount++;
		else if (line[i] == '}')
			closeCount++;
	}

	if (openCount > 1 || closeCount > 1       // If we have something like "server {{{{{"
	    || (openCount > 0 && closeCount > 0)) // If we have something like "server }{"
	{
		std::cerr << "More than one bracket in the same line" << std::endl;
		return false;
	}

	if (closeCount > 0)
	{
		if (state == true)
			state = false;

		else
		{
			std::cerr << "In configuration, is trying to close when its already closed" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	else if (openCount > 0)
	{
		if (state == false)
			state = true;

		else
		{
			std::cerr << "In configuration, is trying to open when its already open" << std::endl;
			return false; // Return false, if something the state is the same
		}
	}

	return true;
}

// This method is to check especific cases like " {{{ location / }}}"
// This way we split the string by the location and see if we open him after
// Only for location cases
bool checkSplitString(const std::string &line, const std::string &sep, bool &isServerOpen)
{
	std::string left = "";
	std::string right = "";
	std::string empty = "";

	std::size_t pos = line.find(sep);
	if (pos == std::string::npos)
		return true;

	left = line.substr(0, pos);
	right = line.substr(pos + sep.size());

	if (isServerOpen == true) // Since its already open, checks only the left
	{
		if (containBrackets(left, isServerOpen, empty) == false)
			return false;
		return true;
	}

	if (isServerOpen == false) // Checks the right side if it is open in the same line " location /upload {"
	{
		if (containBrackets(right, isServerOpen, empty) == false)
			return false;
	}

	return true;
}

void print(const char *src)
{

	std::string a(src);
	std::cout << a << std::endl;
};

static std::string GetExtension(const std::string &path)
{
	size_t slashPos = path.find_last_of('/');
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
		return "";

	if (slashPos != std::string::npos && dotPos < slashPos)
		return "";

	return path.substr(dotPos);
}
File *getMatchFile(const std::string &path,  std::vector<File> &files)
{
	std::string ext = GetExtension(path); // ".txt", ".py", etc.
	if (ext.empty())
		return NULL;

	for (size_t i = 0; i < files.size(); ++i)
	{
		if (files[i].extension.find(ext) != std::string::npos)
		{
			return &files[i];
		}
	}
	return NULL;
}

const Location *getMatchLocation(const std::string &path, const std::vector<Location> &locations)
{

	const Location *matchedLocation = NULL;
	size_t matchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{

		const std::string &locPath = locations[i].path;
		// if (path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > matchLength)
		if (path.find(locPath) != std::string::npos && locPath.size() > matchLength)
		{
			matchedLocation = &locations[i];
			matchLength = locPath.size();
		}
		//! Why here??
		// Checks for the correct CGI location, if we don't find the right location, we return the last one found it
		// if ( matchedLocation != NULL && matchedLocation->cgi.empty() == false ) {
		// 	// Loops into the vector until we find the Extension cgi, for example ".py"
		// 	std::vector< std::string > cgiExtensions = matchedLocation->cgi_extension;
		// 	if ( std::find( cgiExtensions.begin(), cgiExtensions.end(), GetExtension( path ) ) !=
		// 	     cgiExtensions.end() ) {
		// 		return matchedLocation;
		// 	}
		// }
	}
	return (matchedLocation);
}