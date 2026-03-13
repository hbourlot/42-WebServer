// #include "webserver.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define MAGENTA "\033[35m"
#define BLUE "\033[34m"
#define GREEN "\033[32m"

void printHttpHeaders( const std::map< std::string, std::string > &headers ) {
	for ( std::map< std::string, std::string >::const_iterator it = headers.begin(); it != headers.end(); ++it ) {
		std::cout << it->first << ": " << it->second << "\n";
	}
}

void PrintFiles( std::vector< File > &files ) {
	if ( files.empty() )
		return;

	std::cout << MAGENTA << "  FILES CONFIGURATION:" << RESET << std::endl;

	for ( size_t i = 0; i < files.size(); i++ ) {
		File &file = files[i];
		std::cout << "    [File]: " << BOLD << GREEN << file.extension << RESET << std::endl;

		std::cout << "      ├─ Methods: [ ";
		for ( size_t j = 0; j < file.methods.size(); j++ )
			std::cout << file.methods[j] << ( j == file.methods.size() - 1 ? "" : ", " );
		std::cout << " ]" << std::endl;

		std::cout << "      ├─ Root: " << ( file.root.empty() ? "(default)" : file.root ) << std::endl;
		std::cout << "      ├─ Index: " << ( file.index.empty() ? "(none)" : file.index ) << std::endl;
		std::cout << "      ├─ CGI Pass: " << ( file.cgi_pass.empty() ? "Disabled" : file.cgi_pass ) << std::endl;

		std::cout << "      └─ Max Buffer: " << file.max_buffer_size << " bytes" << std::endl;
		std::cout << std::endl;
	}
}

void PrintLocations( std::vector< Directory > &locations ) {
	for ( size_t i = 0; i < locations.size(); i++ ) {
		Directory &loc = locations[i];
		std::cout << YELLOW << "    [Location " << i << "]: " << BOLD << loc.path << RESET << std::endl;

		std::cout << "      ├─ Root: " << loc.root << std::endl;
		std::cout << "      ├─ Index: " << loc.index << std::endl;
		std::cout << "      ├─ Methods: [ ";
		for ( size_t j = 0; j < loc.methods.size(); j++ )
			std::cout << loc.methods[j] << ( j == loc.methods.size() - 1 ? "" : ", " );
		std::cout << " ]" << std::endl;

		if ( !loc.redirection.empty() )
			std::cout << "      ├─ Redirection: " << loc.redirection << std::endl;

		std::cout << "      ├─ CGI Pass: " << ( loc.cgi_pass.empty() ? "None" : loc.cgi_pass ) << std::endl;

		if ( !loc.cgi.empty() ) {
			std::cout << "      ├─ CGI Mappings:" << std::endl;
			for ( std::map< std::string, std::string >::iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it ) {
				std::cout << "      │   └─ " << it->first << " => " << it->second << std::endl;
			}
		}

		std::cout << "      └─ Limits: Body=" << loc.max_body_size << " | Buffer=" << loc.max_buffer_size << std::endl;
		std::cout << std::endl;
	}
}

void PrintConfigs( Configs &configs ) {
	std::cout << BOLD << MAGENTA << "\n================ CONFIGURATION DUMP ================" << RESET << std::endl;

	for ( size_t i = 0; i < configs.servers.size(); i++ ) {
		ServerConfig &server = configs.servers[i];
		std::cout << BLUE << "SERVER [" << i << "]" << RESET << std::endl;
		std::cout << "  HOST: " << BOLD << server.host << RESET << " | PORT: " << BOLD << server.port[0] << RESET
				  << std::endl;
		std::cout << "  NAME: " << ( server.serverName.empty() ? "(none)" : server.serverName ) << std::endl;
		std::cout << "  ROOT: " << server.root << std::endl;
		std::cout << "  MAX_BUFFER_SIZE:  " << server.max_buffer_size << std::endl;
		std::cout << "  MAX_BODY_SIZE:  " << server.max_body_size << std::endl;
		// Páginas de Erro formatadas
		std::cout << "  ERROR PAGES: ";
		if ( server.errorPage.empty() )
			std::cout << "Default";
		for ( std::map< int, std::string >::iterator it = server.errorPage.begin(); it != server.errorPage.end();
			  ++it ) {
			std::cout << "[" << it->first << " -> " << it->second << "] ";
		}
		std::cout << "\n  TIMEOUT: " << server.alive_timeout << "s" << std::endl;

		std::cout << CYAN << "  DIRECTORIES:" << RESET << std::endl;

		for ( std::map< std::string, std::vector< int > >::iterator it = configs.serversPort.begin();
			  it != configs.serversPort.end(); ++it ) {
			std::cout << "[" << it->first << " -> ";

			for ( size_t i = 0; i < it->second.size(); i++ ) {
				std::cout << it->second[i];

				if ( i < it->second.size() - 1 )
					std::cout << ", ";
			}

			std::cout << "] ";
		}

		std::cout << std::endl;
		PrintLocations( server.directories );
		PrintFiles( server.files );
		std::cout << BOLD << MAGENTA << "----------------------------------------------------" << RESET << std::endl;
	}
}
