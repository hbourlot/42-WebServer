#include "http_tcpServerException_linux.hpp"

// template < typename T> 
 
std::map<std::string, std::string> jsonForm(std::string body)
{
    std::map<std::string, std::string> jsonform;
    std::istringstream coma(body);
    std::string line;
    while(std::getline(coma, line, ','))
        std::cout << line;

}