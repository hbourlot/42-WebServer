#include "../../inc/webserver.hpp"


std::string ft_strtrim(const std::string &str)
{
    unsigned int start = 0;
    unsigned int end = str.length();
    // std::cout << std::isspace('\r');

    while(start < str.length() && std::isspace(str[start]))
            start++;
    while(end > start && std::isspace(str[end-1]))
        end--;
    return(str.substr(start,end -start));
}