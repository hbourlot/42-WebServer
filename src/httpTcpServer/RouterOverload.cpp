#include "httpTcpServer/HttpTcpServerLinux.hpp"
#include <algorithm>


static std::string getFileName(const std::string &path) {
 
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
        return path; 

    return path.substr(pos + 1); 
}

void http::Router::routeStaticRequest( Client &client, const ServerConfig &server, const File &file ) {
	http::Request &request = client.getRequest();

	if ( request.method == "GET" ) {
		handleGet( client, server, file );
	} else if ( request.method == "POST" ) {
		handlePost( client, server, file );
	} else if ( request.method == "DELETE" ) {
		handleDelete( client, server, file );
	} else {
		client.getResponse().buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
}

void http::Router::handleGet( Client &client, const ServerConfig &server, const File &file ) {
	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	std::string filePath = file.root + "/" + getFileName( request.path );

	if ( !std::ifstream( filePath.c_str() ).is_open() ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		return;
	}

	response.buildFileResponse( HTTP_OK, filePath, server );
}

void http::Router::handlePost( Client &client, const ServerConfig &server, const File &file ) {
	http::Response &response = client.getResponse();

	if ( std::find( file.methods.begin(), file.methods.end(), "POST" ) != file.methods.end() ) {
		response.buildErrorResponse( HTTP_NOT_IMPLEMENTED, server );
	} else {
		response.buildErrorResponse( HTTP_FORBID_METHOD, server );
	}
}

void http::Router::handleDelete( Client &client, const ServerConfig &server, const File &file ) {
	http::Response &response = client.getResponse();
	std::string filePath = file.root + "/" + getFileName( client.getRequest().path );

	struct stat st;
	if ( stat( filePath.c_str(), &st ) != 0 ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		return;
	}

	if ( remove( filePath.c_str() ) != 0 ) {
		response.buildErrorResponse( HTTP_SERVER_ERR, server );
		return;
	}

	response.buildResponse( HTTP_NO_CONTENT, "" );
}
