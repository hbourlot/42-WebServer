#pragma once
#include "Configs.hpp"
#include <iostream>



void debugLocation( const Location &loc ) {
	std::cout << "--- START DEBUG\n";
	std::cout << "Location:" << std::endl;
	std::cout << "  path: " << loc.path << std::endl;
	std::cout << "  methods: ";
	for ( size_t i = 0; i < loc.methods.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.methods[ i ];
	}
	std::cout << std::endl;
	std::cout << "  root: " << loc.root << std::endl;
	std::cout << "  index: " << loc.index << std::endl;
	std::cout << "  redirection: " << loc.redirection << std::endl;
	std::cout << "  cgi_extension: ";
	for ( size_t i = 0; i < loc.cgi_extension.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.cgi_extension[ i ];
	}
	std::cout << std::endl;
	std::cout << "  cgi_path: ";
	for ( size_t i = 0; i < loc.cgi_path.size(); ++i ) {
		if ( i > 0 )
			std::cout << ", ";
		std::cout << loc.cgi_path[ i ];
	}
	std::cout << std::endl;
	std::cout << "  cgi map: ";
	for ( std::map< std::string, std::string >::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it ) {
		std::cout << "[" << it->first << "]=" << it->second << " ";
	}
	std::cout << std::endl;
	std::cout << "  uploadEnable: " << ( loc.uploadEnable ? "true" : "false" ) << std::endl;
	std::cout << "  uploadStore: " << loc.uploadStore << std::endl;
	std::cout << "  autoIndex: " << ( loc.autoIndex ? "true" : "false" ) << std::endl;

	std::cout << "--- END DEBUG\n";
}