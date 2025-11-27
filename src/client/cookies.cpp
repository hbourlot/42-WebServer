#include "Client/ClientEventProcessor.hpp"
#include <ctime>
#include <iomanip>

static std::string getSessionIdFromCookies( std::string cookieHeader ) {
	std::string sessionId;
	size_t pos = cookieHeader.find( "sessionId=" );
	if ( pos == std::string::npos )
		return ( sessionId );

	size_t start = cookieHeader.find( "=", pos ) + 1;

	size_t end = cookieHeader.find( ';', start );
	if ( end == std::string::npos )
		end = cookieHeader.length();

	sessionId = cookieHeader.substr( start, end - start );
	return ( sessionId );
}

static std::string generateSessionId() {
	static bool seeded = false;
	if ( !seeded ) {
		std::srand( std::time( 0 ) );
		seeded = true;
	}

	unsigned int r = std::rand();

	std::stringstream hex;
	hex << std::hex << std::uppercase << std::setw( 8 ) << std::setfill( '0' ) << r;

	return ( hex.str() );
}

void ensureSessionId( Client &client ) {

	std::string cookieHeader;
	if ( client.getRequest().headers.count( "Cookie" ) ) {
		cookieHeader = client.getRequest().headers[ "Cookie" ];
	}

	std::string sessionId;
	if ( !cookieHeader.empty() ) {
		sessionId = getSessionIdFromCookies( cookieHeader );
	}

	if ( sessionId.empty() ) {
		sessionId = generateSessionId();
		client.getResponse().addToHeader( "Set-Cookie", "sessionId=" + sessionId + "; Path=/; HttpOnly" );
	}
	client.setSessionId( sessionId );
}