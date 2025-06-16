#include "http_tcpServer/Http_tcpServer_linux.hpp"
#include <sys/poll.h>
#include <unistd.h>

void http::TcpServer::readRequest(std::vector<pollfd> &fds, int i)
{
	static std::map<int, std::string> buffers; // buffer per fd
	char buffer[BUFFER_SIZE + 1] = {0};
	ssize_t bytesReceived = read(fds[i].fd, buffer, BUFFER_SIZE);
	if (bytesReceived <= 0)
	{
		if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
		{
			std::cerr << "Error: read()\n";
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
		}
		return;
	}
	buffers[fds[i].fd].append(buffer, bytesReceived);
	// Check if we have all headers
	size_t headerEnd = buffers[fds[i].fd].find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return; // Wait for more data
	// Parse headers to get Content-Length
	std::string headers = buffers[fds[i].fd].substr(0, headerEnd + 4);
	size_t contentLength = 0;
	size_t pos = headers.find("Content-Length:");
	if (pos != std::string::npos)
	{
		pos += 15;
		while (pos < headers.size() && isspace(headers[pos]))
			++pos;
		size_t end = headers.find("\r\n", pos);
		contentLength =
		    std::strtoul(headers.substr(pos, end - pos).c_str(), NULL, 10);
	}
	// Check if we have the full body
	size_t totalExpected = headerEnd + 4 + contentLength;
	if (buffers[fds[i].fd].size() < totalExpected)
		return; // Wait for more data
	// Now we have the full request
	std::string requestContent = buffers[fds[i].fd].substr(0, totalExpected);
	parseRequest(_request, requestContent);
	fds[i].events |= POLLOUT;
	buffers.erase(fds[i].fd); // Clean up
}
