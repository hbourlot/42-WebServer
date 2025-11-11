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
	std::cout << RED << "[" << dateString() << "]"
	          << "[ERROR]: " << msg << NC << std::endl;
}

void Logs::log( Level lvl, const std::string &msg ) {
	switch ( lvl ) {
	case INFO:
		info( msg );
		break;
	case WARN:
		warning( msg );
		break;
	case ERROR:
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
	log( ERROR, ss.str() );
}