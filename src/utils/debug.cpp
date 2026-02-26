// #include "webserver.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"

void printHttpHeaders( const std::map< std::string, std::string > &headers ) {
	for ( std::map< std::string, std::string >::const_iterator it = headers.begin(); it != headers.end(); ++it ) {
		std::cout << it->first << ": " << it->second << "\n";
	}
}

void PrintFiles(std::vector<File> &files)
{
	for (int i = 0; i < files.size(); i++)
	{
		File file = files[i];
		std::cout << "----- PRINTING FILE " << file.extension << " -----" << std::endl;
		for (int i = 0; i < file.methods.size() ; i++)
			std::cout << "--> Methods: " << file.methods[i] << std::endl;
		std::cout << "Root: " << file.root << std::endl;
		std::cout << "Index: " << file.index << std::endl;
		std::cout << "Cgi pass: " << file.cgi_pass << std::endl;
		std::cout << "Max buffer size: " << file.max_buffer_size << std::endl;

	}
}

void PrintLocations(std::vector< Location > &locations)
{
	for (int i = 0; i < locations.size(); i++)
	{
		Location location = locations[i];
		std::cout << "----- PRINTING LOCATION " << location.extension << " -----" << std::endl;
		std::cout << "Path: " << location.path << std::endl;
		for (int i = 0; i < location.methods.size() ; i++)
			std::cout << "--> Methods: " << location.methods[i] << std::endl;
		std::cout << "Root: " << location.root << std::endl;
		std::cout << "Index: " << location.index << std::endl;
		std::cout << "Auto index: " << location.autoIndex << std::endl;
		std::cout << "Redirection: " << location.redirection << std::endl;
		std::cout << "Cgi pass: " << location.cgi_pass << std::endl;
		
		for (int i = 0; i < location.cgi_extension.size() ; i++)
			std::cout << "--> Cgi extension: " << location.cgi_extension[i] << std::endl;

		for (int i = 0; i < location.cgi_path.size() ; i++)
			std::cout << "--> Cgi path: " << location.cgi_path[i] << std::endl;
	
		for (std::map<std::string, std::string>::iterator it = location.cgi.begin(); it != location.cgi.end(); ++it) {
			std::cout  << "--> Cgi first: " << it->first << " --> Cgi second: " << it->second << std::endl;
    	}

		std::cout << "Upload enabled: " << location.uploadEnable << std::endl;
		std::cout << "Upload store: " << location.uploadStore << std::endl;
		std::cout << "Max body size: " << location.max_body_size << std::endl;
		std::cout << "Max buffer size: " << location.max_buffer_size << std::endl;

		std::cout << "\n\n\n\n" << std::endl;
	}
}

void PrintConfigs(Configs &configs)
{
	for (int i = 0; i < configs.servers.size(); i++)
	{
		ServerConfig server = configs.servers[i];
		std::cout << "----- PRINTING SERVER INFO -----" << std::endl;
		std::cout << "Root: " << server.root << std::endl;
		std::cout << "Index: " << server.index << std::endl;
		std::cout << "Host: " << server.host << std::endl;
		std::cout << "temp_path: " << server.temp_path << std::endl;
		std::cout << "Port: " << server.port << std::endl;
		std::cout << "Server Name: " << server.serverName << std::endl;
		for (std::map<int, std::string>::iterator it = server.errorPage.begin(); it != server.errorPage.end(); ++it) {
			std::cout  << "--> Error pages: " << it->second << std::endl;
    	}
		std::cout << "Alive timeout: " << server.alive_timeout << std::endl;
		PrintLocations(server.locations);
		// PrintFiles(server.files);
		std::cout << "\n\n\n";
	}
}

// void printLocation( const Location &location ) {
// 	// std::cout << << std::endl;
// 	std::cout << "location.path " << location.path << std::endl;
// 	std::cout << "location.root " << location.root << std::endl;
// 	std::cout << "location.redirection " << location.redirection << std::endl;
// 	std::cout << "location.uploadEnable " << location.uploadEnable << std::endl;
// 	std::cout << "location.uploadStore " << location.uploadStore << std::endl;
// 	std::cout << "location.autoIndex " << location.autoIndex << std::endl;
// 	std::cout << "location.index " << location.index << std::endl;
// }

// static void printEnvStrings(std::vector< std::string > &_envStrings) {
// 	std::cerr << "ENVSTRINGS:" << std::endl;
// 	for (size_t i = 0; i < _envStrings.size(); ++i) {
// 		std::cerr << "  [" << i << "]: " << _envStrings[i] << std::endl;
// 	}
// 	std::cerr << "END ENVSTRINGS" << std::endl;
// }

// static void debugCgiExec(const char *filePath, char *const argv[], char *const envp[]) {
// 	std::cerr << "CGI EXEC DEBUG" << std::endl;
// 	std::cerr << "File path: " << (filePath ? filePath : "NULL") << std::endl;

// 	std::cerr << "ARGV:" << std::endl;
// 	for (int i = 0; argv && argv[i]; ++i) {
// 		std::cerr << "  argv[" << i << "]: " << argv[i] << std::endl;
// 	}

// 	std::cerr << "ENVP:" << std::endl;
// 	for (int i = 0; envp && envp[i]; ++i) {
// 		std::cerr << "  envp[" << i << "]: " << envp[i] << std::endl;
// 	}
// 	std::cerr << "END DEBUG" << std::endl;
// }

// void static printConfigs( Configs &config ) {
// 	std::vector< ServerConfig >::iterator it = config.servers.begin();
// 	std::vector< ServerConfig >::iterator ite = config.servers.end();
// 	int i = 1;
// 	while ( it != ite ) {

// 		std::cout << "\033[31m";
// 		std::cout << "***************************************" << std::endl;
// 		std::cout << "***********PRINTING SERVER " << i << "***********" << std::endl;
// 		std::cout << "***************************************\n" << std::endl;
// 		std::cout << "\033[0m";

// 		std::cout << "Host: " << it->host << std::endl;
// 		std::cout << "Port " << it->port << std::endl;
// 		std::cout << "Server_Name: " << it->serverName << std::endl;
// 		std::cout << "Max request: " << it->max_body_size << std::endl;

// 		for ( std::map< int, std::string >::iterator ep_it = it->errorPage.begin(); ep_it != it->errorPage.end();
// 		      ++ep_it ) {
// 			std::cout << "Code: " << ep_it->first << " | Path: " << ep_it->second << std::endl;
// 		}

// 		for ( std::vector< Location >::iterator itl = it->locations.begin(); itl != it->locations.end(); ++itl ) {
// 			std::cout << "  Location Path: " << itl->path << "|" << std::endl;
// 			std::cout << "  Root: " << itl->root << std::endl;
// 			std::cout << "  Redirection: " << itl->redirection << std::endl;
// 			std::cout << "  Methods: ";
// 			for ( int i = 0; itl->methods.size() > i; i++ )
// 				std::cout << itl->methods[ i ] << " ";
// 			std::cout << "\n";
// 			if ( !itl->cgi.empty() ) {
// 				std::cout << "      CGI INFORMATION: " << std::endl;
// 				for ( std::map< std::string, std::string >::iterator itm = itl->cgi.begin(); itm != itl->cgi.end();
// 				      itm++ ) {
// 					std::cout << "               Extension: " << itm->first << " | Paths: " << itm->second << std::endl;
// 				}
// 			}

// 			std::cout << "  Upload_Enable: " << itl->uploadEnable << std::endl;
// 			std::cout << "  Upload_Store: " << itl->uploadStore << std::endl;
// 			std::cout << "  Autoindex: " << itl->autoIndex << std::endl;
// 			std::cout << "\n\n";
// 		}
// 		// for (std::vector<int, std::string>::iterator it3 =
// 		// it->errorPage.begin(); it3 != (*it)->errorPage.end(); it++)

// 		it++;
// 		i++;
// 		std::cout << "\n";
// 	}
// }