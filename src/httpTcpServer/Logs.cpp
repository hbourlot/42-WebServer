#include "Logs/Logs.hpp"

void Logs::info( const std::string &msg ) {
	std::cout << BLUE << "[" << dateString() << "]"
	          << "[INFO]: " << msg << NC << std::endl;
}
void Logs::warning( const std::string &msg ) {
	std::cout << YELLOW << "[" << dateString() << "]"
	          << "[WARNING]: " << msg << NC << std::endl;
}
void Logs::error( const std::string &msg ) {
	std::cerr << RED << "[" << dateString() << "]"
	          << "[ERROR]: " << msg << NC << std::endl;
}

void Logs::log( Level lvl, const std::string &msg ) {
	switch ( lvl ) {
	case LOGS_INFO:
		info( msg );
		break;
	case LOGS_WARN:
		warning( msg );
		break;
	case LOGS_ERROR:
		error( msg );
		break;

	default:
		break;
	}
}

void Logs::logAcceptError( const sockaddr_in &socketAddress ) {
	std::ostringstream ss;
	ss << "Server failed to accept incoming connection from =>\n"
	   << "[ADDRESS: " << inet_ntoa( socketAddress.sin_addr ) << "]\n"
	   << "[PORT: " << ntohs( socketAddress.sin_port ) << "]\n";
	log( LOGS_ERROR, ss.str() );
}