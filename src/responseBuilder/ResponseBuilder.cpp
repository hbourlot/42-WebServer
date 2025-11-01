#include "httpTcpServer/HttpTcpServerLinux.hpp"

httpResponse ResponseBuilder::buildResponse( const HttpStatusCode &status, const std::string &body,
                                             const std::string &headerKey, const std::string &headerValue,
                                             httpRequest *req ) {
	httpResponse res;
	res.statusCode = status.code;
	res.statusMsg = status.message;
	res.body = body;

	if ( !headerKey.empty() )
		res.addToHeader( headerKey, headerValue );

	if ( req )
		res.setDefaultHeaders( *req );
	else
		res.setDefaultHeaders();

	return res;
}

httpResponse ResponseBuilder::buildErrorResponse( const HttpStatusCode &status ) {
	std::string body = status.message + " (" + status.code + ")";
	return buildResponse( status, body, "Content-Type", "text/plain" );
}
httpResponse ResponseBuilder::buildRedirect( const HttpStatusCode &status, const std::string &url ) {
	return buildResponse( status, "", "Location", url );
}

httpResponse ResponseBuilder::buildFileResponse( const HttpStatusCode &status, const std::string &filePath,
                                                 const ServerConfig &server, bool isError ) {
	std::string content = readFileContent( filePath );
	if ( content.empty() ) {
		if ( !isError )
			return buildFileResponse( HTTP_NOT_FOUND, server.errorPage.at( 404 ), server, true );
		else
			return buildErrorResponse( status );
	}
	return buildResponse( status, content, "Content-Type", getContentType( filePath ) );
}

static size_t getMetadataEstimate(const size_t fileSize ) {
	if ( fileSize < 100 * 1024 * 1024 ) {                                 // <100MB
		return std::min( fileSize, static_cast< size_t >( 32 * 1024 ) );  // 32KB max
	} else if ( fileSize < 1024 * 1024 * 1024 ) {                         // <1GB
		return std::min( fileSize, static_cast< size_t >( 128 * 1024 ) ); // 128KB max
	} else {                                                              // >1GB
		return std::min( fileSize, static_cast< size_t >( 512 * 1024 ) ); // 512KB max
	}
}

// httpResponse ResponseBuilder::buildFileResponse(const HttpStatusCode &status, const std::string &filePath,
//                                                 const ServerConfig &server, httpRequest* req)
// {

// 	struct stat fileStat;
// 	if (stat(filePath.c_str(), &fileStat) != 0)
// 		return buildErrorResponse(HTTP_NOT_FOUND);

// 	size_t fileSize = getMetadataEstimate(fileStat.st_size);


// 	// Case bigger
// 	if (fileSize > 100 * 1024 * 1024) {

// 		// Needs to check if browser sent Range Request
// 		if (req && req->headers.count("Range")) {
// 			// return buildRangeResponse(filePath, req->headers["Range"], fileSize);
// 		}
// 		// Browser wants full file - Accept-Range header
// 		return buildLargeFileResponse(filePath, fileSize);
// 	}


// 	std::string content = readFileContent(filePath);
// 	if (content.empty())
// 	{
// 		if (!isError)
// 			return buildFileResponse(HTTP_NOT_FOUND, server.errorPage.at(404), server, true);
// 		else
// 			return buildErrorResponse(status);
// 	}

// 	return buildResponse(status, content, "Content-Type", getContentType(filePath));
// }

std::string ResponseBuilder::readFileContent( const std::string &filePath ) {
	std::ifstream file( filePath.c_str() );
	if ( !file.is_open() )
		return "";

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

std::string ResponseBuilder::getContentType( const std::string &filePath ) {
	size_t dot = filePath.find_last_of( '.' );
	if ( dot == std::string::npos )
		return "application/octet-stream"; // binario genérico

	std::string ext = filePath.substr( dot + 1 );
	if ( ext == "html" || ext == "htm" )
		return "text/html";
	if ( ext == "css" )
		return "text/css";
	if ( ext == "png" )
		return "image/png";
	if ( ext == "jpg" || ext == "jpeg" )
		return "image/jpeg";
	if ( ext == "gif" )
		return "image/gif";
	if ( ext == "txt" )
		return "text/plain";
	if ( ext == "pdf" )
		return "application/pdf";
	return "application/octet-stream";
}
