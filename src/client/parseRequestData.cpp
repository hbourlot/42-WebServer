#include "Client/ClientEventProcessor.hpp"
#include <algorithm>

static void parseRequestQueries(http::Request &request) {
	std::string fullPath = request.getUri();
	std::string::size_type qpos = fullPath.find('?');

	if (qpos != std::string::npos) {
		request.setQueryString(fullPath.substr(qpos + 1));
		request.setUri(fullPath.substr(0, qpos));
	} else {
		request.setQueryString("");
	}
}

static void parsePath(http::Request &request, const ServerConfig &serverInfo) {
	std::string &path = request.getUri();

	// by default
	request.setPathInfo("");
	request.setPathTranslated("");

	for (size_t i = 0; i < serverInfo.directories.size(); ++i) {
		const Directory &loc = serverInfo.directories[i];
		if (loc.cgi_extension.empty())
			continue;

		for (size_t j = 0; j < loc.cgi_extension.size(); ++j) {
			const std::string &ext = loc.cgi_extension[j];

			size_t pos = path.find(ext);
			if (pos == std::string::npos)
				continue;

			if (pos + ext.size() < path.size() && path[pos + ext.size()] != '/')
				continue;

			// PATH_INFO = whatever that comes after *.cgi/
			if (pos + ext.size() < path.size())
				request.setPathInfo(path.substr(pos + ext.size()));
			else
				request.setPathInfo("");

			// PATH_TRANSLATED = root + PATH_INFO
			request.setPathTranslated(loc.root);
			if (!request.getPathInfo().empty() && request.getPathInfo()[0] == '/')
				request.getPathTranslated() += request.getPathInfo();
			else if (!request.getPathInfo().empty())
				request.getPathTranslated() += "/" + request.getPathInfo();

			// Adjust request.path
			path = path.substr(0, pos + ext.size());

			return;
		}
	}
}
static void decodeURI(std::string &uri) {
	if (uri.empty())
		return;

	std::string toReplace = "%20";
	size_t pos = uri.find(toReplace);
	while (pos != std::string::npos) {
		uri.replace(pos, 3, " ");
		pos = uri.find(toReplace, pos + 1);
	}
}

static bool parseRequestLine(http::Request &req, const ServerConfig &serverInfo, const std::string &readBuffer,
                             size_t lineEnd) {

	const char *data = readBuffer.c_str();

	std::string requestLine(data, lineEnd);

	size_t posM = requestLine.find(' ');
	size_t posP = requestLine.find(' ', posM + 1);
	if (posM == std::string::npos || posP == std::string::npos)
		return false;

	req.setMethod(requestLine.substr(0, posM));

	req.setUri(requestLine.substr(posM + 1, posP - posM - 1));
	decodeURI(req.getUri());
	// std::cout << "req.getUri():" << req.getUri() << std::endl;

	req.setServerProtocol(requestLine.substr(posP + 1));

	req.setMatchLocation(getMatchLocation(req.getUri(), serverInfo.locations));
	req.setFileDirectory(getMatchDirectory(req.getUri(), serverInfo.directories));

	return true;
}

#include <limits.h>
static void parseRequestHeaders(http::Request &req, const std::string &readBuffer, size_t headerEnd) {
	const char *data = readBuffer.c_str();
	size_t pos = 0;
	while (pos < headerEnd) {
		size_t lineEnd = readBuffer.find("\r\n", pos);
		if (lineEnd == std::string::npos || lineEnd == pos)
			break;

		std::string line(data + pos, lineEnd - pos);
		pos = lineEnd + 2;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = ft_strtrim(line.substr(0, colon));
		std::string value = ft_strtrim(line.substr(colon + 1));
		req.getHeaders()[key] = value;
	}
}

static void discardingBody(Client &client) {
	size_t available = client.getReadBuffer().size();
	size_t bytesToDiscard = client.getBytesToDiscard();

	if (available >= bytesToDiscard) {
		client.consumeReadBuffer(bytesToDiscard);
		client.setBytesToDiscard(0);
		client.setDiscardingBody(false);

		client.getResponse().initFromRequest(client.getRequest());
		client.setState(PARSE_TOO_LARGE);

	} else {
		client.setBytesToDiscard(bytesToDiscard - available);
		client.clearReadBuffer();
	}
}

static bool parseContentLengthBody(Client &client, const ServerConfig &configs) {
	http::Request &request = client.getRequest();
	std::string &buffer = client.getReadBuffer();
	CLengthParser &lengthParser = request.getLengthParser();

	if (!lengthParser.hasLength) {
		lengthParser.length = strtoul(request.getHeaders()["Content-Length"].c_str(), NULL, 10);
		lengthParser.hasLength = true;

		if (lengthParser.length > request.getMatchLocation()->max_body_size) {
			client.setDiscardingBody(true);
			client.setBytesToDiscard(lengthParser.length);
		}
	}

	size_t remaining = lengthParser.length - lengthParser.bytesRead;
	size_t canRead = std::min(buffer.size(), remaining);

	if (canRead == 0 && lengthParser.length > 0)
		return false;

	if (client.getDiscardingBody()) {
		lengthParser.bytesRead += canRead;
		buffer.erase(0, canRead);
		discardingBody(client);
		return (lengthParser.bytesRead >= lengthParser.length);
	} else {
		if (request.appendBody(buffer.data(), canRead, configs)) {
			client.setState(PARSE_ERROR);
			return true;
		}

		lengthParser.bytesRead += canRead;
		buffer.erase(0, canRead);

		if (lengthParser.bytesRead >= lengthParser.length) {
			return true;
		}
	}

	return false;
}

static bool parseChunkBody(Client &client, const ServerConfig &configs) {
	std::string &buffer = client.getReadBuffer();
	ChunkParser &chunk = client.getRequest().getChunkParser();
	http::Request &request = client.getRequest();
	while (!buffer.empty()) {
		switch (chunk.state) {
		case CHUNK_SIZE: {
			size_t lineEnd = buffer.find("\r\n");
			if (lineEnd == std::string::npos)
				return false;

			std::string sizeStr = buffer.substr(0, lineEnd);
			chunk.currentChunkSize = strtoul(sizeStr.c_str(), NULL, 16);
			buffer.erase(0, lineEnd + 2);
			if (!client.getDiscardingBody() &&
			    (request.getBodySize() + chunk.currentChunkSize > request.getMatchLocation()->max_body_size)) {
				client.setDiscardingBody(true);
			}
			if (chunk.currentChunkSize == 0)
				chunk.state = CHUNK_DONE;
			else {
				chunk.state = CHUNK_DATA;
				chunk.bytesReadInChunk = 0;
			}
			break;
		}
		case CHUNK_DATA: {
			size_t remaining = chunk.currentChunkSize - chunk.bytesReadInChunk;
			size_t canRead = std::min(remaining, buffer.size());

			if (client.getDiscardingBody()) {
				buffer.erase(0, canRead);
			} else {
				if (request.appendBody(buffer.c_str(), canRead, configs)) {
					client.setState(PARSE_ERROR);
					return true;
				}
				buffer.erase(0, canRead);
			}
			chunk.bytesReadInChunk += canRead;
			if (chunk.bytesReadInChunk == chunk.currentChunkSize)
				chunk.state = CHUNK_CRLF;
			else
				return false;
			break;
		}
		case CHUNK_CRLF: {
			if (buffer.size() < 2)
				return false;
			buffer.erase(0, 2);
			chunk.state = CHUNK_SIZE;
			break;
		}
		case CHUNK_DONE: {
			if (buffer.size() < 2)
				return false;
			buffer.erase(0, 2);

			if (client.getDiscardingBody()) {
				client.setState(PARSE_TOO_LARGE);
			}
			return (true);
		}
		}
	}
	return false;
}

static bool parseRequestBody(Client &client, const ServerConfig &configs) {

	http::Request &req = client.getRequest();

	if (req.getHeaders().count("Transfer-Encoding") && req.getHeaders()["Transfer-Encoding"] == "chunked")
		return parseChunkBody(client, configs);

	if (req.getHeaders().count("Content-Length"))
		return (parseContentLengthBody(client, configs));

	req.getBody().clear();

	return true;
}

bool http::ClientEventProcessor::parseRequestData(Client &client, const ServerConfig &serverInfo) {

	std::string &readBuffer = client.getReadBuffer();
	http::Request &clientRequest = client.getRequest();

	if (clientRequest.getRequestPhase() == START) {
		size_t lineEnd = readBuffer.find("\r\n");
		if (lineEnd == std::string::npos) {
			client.setState(PARSE_INCOMPLETE);
			return false;
		}

		if (!parseRequestLine(clientRequest, serverInfo, readBuffer, lineEnd)) {
			client.setState(PARSE_INCOMPLETE);
			return false;
		}

		parseRequestQueries(clientRequest);
		parsePath(clientRequest, serverInfo);
		readBuffer.erase(0, lineEnd + 2);
		clientRequest.setRequestPhase(HEADER);
	}

	if (clientRequest.getRequestPhase() == HEADER) {
		size_t headerEnd = readBuffer.find("\r\n\r\n");
		if (headerEnd == std::string::npos) {
			client.setState(PARSE_INCOMPLETE);
			return false;
		}

		parseRequestHeaders(clientRequest, readBuffer, headerEnd);
		readBuffer.erase(0, headerEnd + 4);
		clientRequest.setRequestPhase(BODY);
	}

	if (clientRequest.getRequestPhase() == BODY) {

		if (!parseRequestBody(client, serverInfo)) {
			client.setState(PARSE_INCOMPLETE);
			return false;
		}

		clientRequest.setRequestPhase(FINISHED);
	}

	if (clientRequest.getRequestPhase() == FINISHED) {

		client.getResponse().initFromRequest(client.getRequest());
		clientRequest.resetChunkParser();
		Logs::log(LOGS_INFO, "Client: " + ft_to_string(client.getFd()) + " Made a Request");
		if (client.getState() != PARSE_ERROR && client.getState() != PARSE_TOO_LARGE)
			client.setState(PARSE_OK);
		clientRequest.setRequestPhase(START);

		return true;
	}
	return false;
}
