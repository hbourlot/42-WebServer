#include "Upload/UploadManager.hpp"

bool UploadManager::handleUpload( const Location &location, Client &client, const ServerConfig &serverInfo ) {
	std::string contentType;

	contentType = client.getRequest().headers.at( "Content-Type" );

	std::cout << contentType << std::endl;

	if ( contentType.find( "multipart/form-data;" ) != std::string::npos ) {
		if ( UploadManager::parseMultipart( location, client, serverInfo ) )
			return ( true );
	} else if ( contentType.find( "text/plain" ) != std::string::npos ) {
	} else {
		client.getResponse().buildErrorResponse( HTTP_UNSUPPORTED_MEDIA, serverInfo );
	}
	return ( false );
}
