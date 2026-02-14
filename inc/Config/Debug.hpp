#pragma once
#include "Configs.hpp"
#include <iostream>

void debugDirectory( const Directory &loc ) {
	std::cout << "--- START DEBUG\n";
	std::cout << "Location:" << std::endl;
	std::cout << "  path: " << loc.path << std::endl;
	std::cout << "  methods: ";
	for ( size_t i = 0; i < loc.methods.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.methods[i];
	}
	std::cout << std::endl;
	std::cout << "  root: " << loc.root << std::endl;
	std::cout << "  index: " << loc.index << std::endl;
	std::cout << "  redirection: " << loc.redirection << std::endl;
	std::cout << "  cgi_extension: ";
	for ( size_t i = 0; i < loc.cgi_extension.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.cgi_extension[i];
	}
	std::cout << std::endl;
	std::cout << "  cgi_path: ";
	for ( size_t i = 0; i < loc.cgi_path.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.cgi_path[i];
	}
	std::cout << std::endl;
	std::cout << "  cgi map: ";
	for ( std::map< std::string, std::string >::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it ) {
		std::cout << "[" << it->first << "]=" << it->second << " ";
	}
	std::cout << std::endl;
	std::cout << "  cgi_pass: " << loc.cgi_pass << std::endl;
	std::cout << "  max_body_size: " << loc.max_body_size << std::endl;
	std::cout << "  max_buffer_size: " << loc.max_buffer_size << std::endl;

	std::cout << "  uploadEnable: " << ( loc.uploadEnable ? "true" : "false" ) << std::endl;
	std::cout << "  uploadStore: " << loc.uploadStore << std::endl;
	std::cout << "  autoIndex: " << ( loc.autoIndex ? "true" : "false" ) << std::endl;

	std::cout << "--- END DEBUG\n";
}

void printDirectories( std::vector< Directory > vec ) {
	for ( int i = 0; i < vec.size(); ++i ) {
		std::cout << "+++++ START +++++\n\n";
		debugDirectory( vec[i] );
		std::cout << "\n+++++ END +++++\n\n";
	}
}

void debugFile( const File &file ) {
	std::cout << "--- START DEBUG\n";
	std::cout << "Location:" << std::endl;
	std::cout << "  extension: " << file.extension << std::endl;
	std::cout << "  methods: ";
	for ( size_t i = 0; i < file.methods.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << file.methods[i];
	}
	std::cout << std::endl;
	std::cout << "  root: " << file.root << std::endl;
	std::cout << "  index: " << file.index << std::endl;
	std::cout << std::endl;
	std::cout << "  cgi_pass: " << file.cgi_pass << std::endl;
	std::cout << "  max_buffer_size: " << file.max_buffer_size << std::endl;

	std::cout << "--- END DEBUG\n";
}

void printFiles( std::vector< File > vec ) {
	for ( int i = 0; i < vec.size(); ++i ) {
		std::cout << "+++++ START +++++\n\n";
		debugFile( vec[i] );
		std::cout << "\n+++++ END +++++\n\n";
	}
}