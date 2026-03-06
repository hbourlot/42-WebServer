#pragma once


#include <sys/stat.h>
#include "CheckConf.hpp"
#include "ConfigUtils.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define HOST 1
#define PORT 2
#define SERVER_NAME 3
#define CLIENT_MAX_BDY 4
#define ERROR_PAGE 5
#define LOCATION 6
#define ROOT 7
#define FILE 8
#define INDEX 9
#define CGIPASS 10
#define TEMP_PATH 11
#define BODY_BUFFER 12
#define ALIVE_TIMEOUT 13
#define METHODS 14
#define REDIRECT 15
#define CGI_EXTENSION 16
#define CGI_PATH 17
#define UPLOAD_ENABLE 18
#define UPLOAD_STORE 19
#define AUTOINDEX 20
#define SERVER 21
#define COMMENT 22
#define EMPTY 23
#define AUTH_LOGIN 24
#define AUTH 25

struct File {
	File();
	std::string extension;
	std::vector< std::string > methods; // method POST GET DELETE
	std::string root;
	std::string index;
	std::string cgi_pass;
	size_t max_body_size;
	size_t max_buffer_size;    // Buffer size to write on temp files
};

struct Directory {
	Directory();
	std::string name;
	std::string path;                   // location --> /upload <--
	std::vector< std::string > methods; // method POST GET DELETE
	std::string root;
	std::string index;
	std::string redirection; // http://example.com;
	std::vector< std::string > cgi_extension;
	std::vector< std::string > cgi_path;
	std::map< std::string, std::string > cgi;
	std::string cgi_pass;
	size_t max_body_size;   // Stores the maximum requests that the client can do
	size_t max_buffer_size; // Buffer size to write on temp files
	bool uploadEnable;
	std::string uploadStore;
	bool autoIndex;
	bool auth;
	std::string auth_login_page;
};

enum LocationType {
	LOCATION_DIR = 1,
	LOCATION_FILE = 2,
	LOCATION_CGI = 4,
	LOCATION_REDIRECT = 8,
	LOCATION_UPLOAD = 16,
};

struct Location {

	Location()
	    : type( 0 ), extension( "" ), path( "" ), methods(), root( "" ), index( "" ), autoIndex( false ),
	      redirection( "" ), cgi_pass( "" ), cgi_extension(), cgi_path(), cgi(), uploadEnable( false ),
	      uploadStore( "" ), max_body_size( 0 ), max_buffer_size( 0 ), auth ( false ), auth_login_page( "/login" ) {
		// SafetyInitialization
	}

	int type;

	// Core routing
	std::string extension;
	std::string path;
	std::vector< std::string > methods;

	// Serving
	std::string root;
	std::string index;
	bool autoIndex;

	// Redirects
	std::string redirection;

	// CGI
	std::string cgi_pass;
	std::vector< std::string > cgi_extension;
	std::vector< std::string > cgi_path;
	std::map< std::string, std::string > cgi;

	// Upload
	bool uploadEnable;
	std::string uploadStore;

	// Limits
	size_t max_body_size;
	size_t max_buffer_size;

	// Session manager
	bool auth;
	std::string auth_login_page;

	// Helper methods
	bool isDir() const {
		return type & LOCATION_DIR;
	}
	bool isFile() const {
		return type & LOCATION_FILE;
	}
	bool isCgi() const {
		return type & LOCATION_CGI;
	}
	bool isRedirect() const {
		return type & LOCATION_REDIRECT;
	}
	bool isUpload() const {
		return type & LOCATION_UPLOAD;
	}
};

struct ServerConfig {
	ServerConfig();
	std::string root;                       // Stores the root path
	std::string index;                      // Stores the index
	std::string host;                       // Stores the host IP
	std::string temp_path;                  // Stores the temp path for temp files
	int port;                               // Stores the port to listen
	std::string serverName;                 // Stores the name server
	std::map< int, std::string > errorPage; // Stores the error pages
	size_t max_body_size;                   // Stores the maximum requests that the client can do
	size_t max_buffer_size;                 // Buffer size to write on temp files
	std::vector< Directory > directories;   // Stores the routes of the HTML pages
	std::vector< File > files;              // Stores the specific files
	std::vector< Location > locations;
	std::string auth_login_page;
	File* GetFileByExtension( std::string extension ); // Get the file by extension ".bat" (For example)
	Directory* GetLocationByPath( std::string path );  // Get the "location '/"path"' "
	size_t alive_timeout;					// Stores the time for max time for a request
};

struct Configs {
	std::vector< ServerConfig > servers; // Will store the servers
};