#include "Upload/UploadManager.hpp"

bool UploadManager::handleUpload( const Location &location, Client &client, const ServerConfig &serverInfo ) {
	std::string contentType;

	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	contentType = client.getRequest().headers.at( "Content-Type" );

	std::cout << contentType << std::endl;

	if ( contentType.find( "multipart/form-data;" ) != std::string::npos ) {
		if ( UploadManager::parseMultipart( location, client, serverInfo ) )
			return ( true );
	} else if ( contentType.find( "text/plain" ) != std::string::npos ) {
	} else {
		response.buildErrorResponse( HTTP_UNSUPPORTED_MEDIA, serverInfo );
	}
	return ( false );
}
