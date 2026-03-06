// #include "webserver.hpp"
#include "httpTcpServer/HttpTcpServerLinux.hpp"
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define BLUE    "\033[34m"
#define GREEN   "\033[32m"

void printHttpHeaders( const std::map< std::string, std::string > &headers ) {
	for ( std::map< std::string, std::string >::const_iterator it = headers.begin(); it != headers.end(); ++it ) {
		std::cout << it->first << ": " << it->second << "\n";
	}
}

void PrintFiles(std::vector<File> &files)
{
    if (files.empty()) return;

    std::cout << MAGENTA << "  FILES CONFIGURATION:" << RESET << std::endl;

    for (size_t i = 0; i < files.size(); i++)
    {
        File &file = files[i];
        // Destaque para a extensão do ficheiro
        std::cout << "    [File]: " << BOLD << GREEN << file.extension << RESET << std::endl;
        
        // Métodos na mesma linha
        std::cout << "      ├─ Methods: [ ";
        for (size_t j = 0; j < file.methods.size(); j++)
            std::cout << file.methods[j] << (j == file.methods.size() - 1 ? "" : ", ");
        std::cout << " ]" << std::endl;

        std::cout << "      ├─ Root: " << (file.root.empty() ? "(default)" : file.root) << std::endl;
        std::cout << "      ├─ Index: " << (file.index.empty() ? "(none)" : file.index) << std::endl;
        std::cout << "      ├─ CGI Pass: " << (file.cgi_pass.empty() ? "Disabled" : file.cgi_pass) << std::endl;
        
        // Footer da secção de ficheiro
        std::cout << "      └─ Max Buffer: " << file.max_buffer_size << " bytes" << std::endl;
        std::cout << std::endl;
    }
}

void PrintLocations(std::vector<Directory> &locations)
{
    for (size_t i = 0; i < locations.size(); i++)
    {
        Directory &loc = locations[i];
        std::cout << YELLOW << "    [Location " << i << "]: " << BOLD << loc.path << RESET << std::endl;
        
        // Usar uma indentação fixa para sub-itens
        std::cout << "      ├─ Root: " << loc.root << std::endl;
        std::cout << "      ├─ Index: " << loc.index << std::endl;
        std::cout << "      ├─ Methods: [ ";
        for (size_t j = 0; j < loc.methods.size(); j++)
            std::cout << loc.methods[j] << (j == loc.methods.size() - 1 ? "" : ", ");
        std::cout << " ]" << std::endl;

        if (!loc.redirection.empty())
            std::cout << "      ├─ Redirection: " << loc.redirection << std::endl;
        
        std::cout << "      ├─ CGI Pass: " << (loc.cgi_pass.empty() ? "None" : loc.cgi_pass) << std::endl;
        
        // Imprimir Mapas de forma limpa
        if (!loc.cgi.empty()) {
            std::cout << "      ├─ CGI Mappings:" << std::endl;
            for (std::map<std::string, std::string>::iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
                std::cout  << "      │   └─ " << it->first << " => " << it->second << std::endl;
            }
        }

        std::cout << "      └─ Limits: Body=" << loc.max_body_size << " | Buffer=" << loc.max_buffer_size << std::endl;
        std::cout << std::endl;
    }
}

void PrintConfigs(Configs &configs)
{
    std::cout << BOLD << MAGENTA << "\n================ CONFIGURATION DUMP ================" << RESET << std::endl;

    for (size_t i = 0; i < configs.servers.size(); i++)
    {
        ServerConfig &server = configs.servers[i];
        std::cout << BLUE << "SERVER [" << i << "]" << RESET << std::endl;
        std::cout << "  HOST: " << BOLD << server.host << RESET << " | PORT: " << BOLD << server.port[0] << RESET << std::endl;
        std::cout << "  NAME: " << (server.serverName.empty() ? "(none)" : server.serverName) << std::endl;
        std::cout << "  ROOT: " << server.root << std::endl;
        std::cout << "  MAX_BUFFER_SIZE:  " << server.max_buffer_size << std::endl;
		std::cout << "  MAX_BODY_SIZE:  " << server.max_body_size << std::endl;
		// Páginas de Erro formatadas
        std::cout << "  ERROR PAGES: ";
        if (server.errorPage.empty()) std::cout << "Default";
        for (std::map<int, std::string>::iterator it = server.errorPage.begin(); it != server.errorPage.end(); ++it) {
            std::cout << "[" << it->first << " -> " << it->second << "] ";
        }
        std::cout << "\n  TIMEOUT: " << server.alive_timeout << "s" << std::endl;

        std::cout << CYAN << "  DIRECTORIES:" << RESET << std::endl;
        
		for (std::map<std::string, std::vector<int> >::iterator it = configs.serversPort.begin(); it != configs.serversPort.end(); ++it)
		{
			std::cout << "[" << it->first << " -> ";

			for (size_t i = 0; i < it->second.size(); i++)
			{
				std::cout << it->second[i];

				if (i < it->second.size() - 1)
					std::cout << ", ";
			}

			std::cout << "] ";
		}

		std::cout << std::endl;
		PrintLocations(server.directories);
        PrintFiles(server.files);
        std::cout << BOLD << MAGENTA << "----------------------------------------------------" << RESET << std::endl;
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