#include "httpTcpServer/HttpTcpServerLinux.hpp"

static std::string getParentPath( const std::string &path ) {
	std::string parent = path;

	if ( !parent.empty() && parent[ parent.length() - 1 ] == '/' )
		parent.erase( parent.length() - 1 );

	size_t prevfolder = parent.find_last_of( '/' );
	if ( prevfolder != std::string::npos && prevfolder != 0 )
		return ( parent.substr( 0, prevfolder ) );

	return ( "/" );
}

static bool hasIndexFile( const std::string &path, const RouteContext &ctx ) {

	if ( ctx.index.empty() )
		return false;

	std::string indexPath = joinPath( path, ctx.index );
	return ( std::ifstream( indexPath.c_str() ).is_open() );
}

static std::string generateAutoIndexPage( const std::string &dirPath, http::Request &request ) {
	std::string html;
	html += "<html>\n";
	html += "  <body>\n";
	html += "    <h1>Index of " + request.path + "</h1>\n";

	DIR *directory = opendir( dirPath.c_str() );
	if ( !directory )
		return "<html><body><h1>Unable to open "
		       "directory</h1></body></html>";

	struct dirent *entry;

	while ( ( entry = readdir( directory ) ) != NULL ) {
		std::string d_name( entry->d_name );
		if ( !d_name.compare( "." ) )
			continue;

		std::string href;

		if ( !d_name.compare( ".." ) )
			href = getParentPath( request.path );
		else
			href = joinPath( request.path, d_name );

		html += "    <p><a href=\"" + href + "\">" + d_name + "</a></p>\n";
	}
	html += "</body>\n";
	html += "</html>";

	closedir( directory );
	return ( html );
}

void http::Router::handleDirectoryListing( Client &client, const ServerConfig &server, const std::string &filePath,
                                           const RouteContext &ctx ) {
	http::Request &request = client.getRequest();
	http::Response &response = client.getResponse();

	if ( hasIndexFile( filePath, ctx ) ) {
		std::string indexPath = joinPath( filePath, ctx.index );
		response.buildFileResponse( HTTP_OK, indexPath, server );
		return;
	}

	if ( !ctx.autoIndex ) {
		response.buildErrorResponse( HTTP_NOT_FOUND, server );
		return;
	}

	std::string body = generateAutoIndexPage( filePath, request );

	response.buildResponse( HTTP_OK, body );

	return;
}