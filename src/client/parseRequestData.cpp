#include "Client/ClientEventProcessor.hpp"

static void parseRequestQueries(http::Request &request) {
	std::string fullPath = request.path;
	std::string::size_type qpos = fullPath.find('?');

	if (qpos != std::string::npos) {
		request.queryString = fullPath.substr(qpos + 1);
		request.path = fullPath.substr(0, qpos);
	} else {
		request.queryString = "";
	}
}

static void parsePath(http::Request &request, const ServerConfig &serverInfo) {
	std::string &path = request.path;

	// by default
	request.pathInfo = "";
	request.pathTranslated = "";

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
				request.pathInfo = path.substr(pos + ext.size());
			else
				request.pathInfo = "";

			// PATH_TRANSLATED = root + PATH_INFO
			request.pathTranslated = loc.root;
			if (!request.pathInfo.empty() && request.pathInfo[0] == '/')
				request.pathTranslated += request.pathInfo;
			else if (!request.pathInfo.empty())
				request.pathTranslated += "/" + request.pathInfo;

			// Adjust request.path
			path = path.substr(0, pos + ext.size());

			return;
		}
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

	req._method = requestLine.substr(0, posM);
	req.path = requestLine.substr(posM + 1, posP - posM - 1);
	req.serverProtocol = requestLine.substr(posP + 1);

	req.matchLocation = getMatchLocation(req.path, serverInfo.locations);
	req.fileDirectory = getMatchDirectory(req.path, serverInfo.directories);
	//! If directory it's removed this can be a Location because its a copy ↑↑↑↑↑

	return true;
}

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
		req.headers[key] = value;
	}
}

static bool parseContentLengthBody(Client &client, const ServerConfig &configs) {
	http::Request &request = client.getRequest();
	std::string &buffer = client.getReadBuffer();

	size_t len = strtoul(request.headers["Content-Length"].c_str(), NULL, 10);
	if (len > buffer.size())
		return false;

	if (request.appendBody(buffer.c_str(), len, configs)) {
		client.setState(PARSE_ERROR);
		return true;
	}
	buffer.erase(0, len);
	return true;
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

			if (request.appendBody(buffer.c_str(), canRead, configs)) {
				client.setState(PARSE_ERROR);
				return true;
			}
			chunk.bytesReadInChunk += canRead;
			buffer.erase(0, canRead);
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
			return (true);
		}
		}
	}
	return false;
}

static bool parseRequestBody(Client &client, const ServerConfig &configs) {

	http::Request &req = client.getRequest();

	if (req.headers.count("Transfer-Encoding") && req.headers["Transfer-Encoding"] == "chunked")
		return parseChunkBody(client, configs);

	if (req.headers.count("Content-Length"))
		return (parseContentLengthBody(client, configs));

	req.body.clear();

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
		ensureSessionId(client);

		clientRequest.resetChunkParser();
		Logs::log(LOGS_INFO, "Client: " + ft_to_string(client.getFd()) + " Made a Request");
		if (client.getState() != PARSE_ERROR) // ! Here for now because line 140
			client.setState(PARSE_OK);
		clientRequest.setRequestPhase(START);

		return true;
	}
	return false;
}
