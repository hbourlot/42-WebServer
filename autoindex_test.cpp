// #include "http_tcpServer/http_tcpServer_linux.hpp"
#include <dirent.h>
#include <cstring>
#include <iostream>
// #include <ifstream>
#include <fstream>

int main() {
    std::string autoindex;
    DIR *directory;
    directory=opendir("./var/www/html");
    struct dirent* dirent;
    dirent = readdir(directory);
    while(dirent != NULL)
    {
        std::cout << dirent->d_name << " " <<dirent->d_off << std::endl;
        std::string d_name(dirent->d_name);
        if(d_name.compare(".") && d_name.compare(".."))
        autoindex += "<a href=" + d_name+ ">" +d_name + "</a>\n";
        dirent = readdir(directory);
    }
    autoindex = "<htlm>\n<body>\n" + autoindex + "</body>\n</html>";
    std::cout << autoindex << std::endl;
    closedir(directory);
    // std::ofstream autoindexfile("./var/www/html/autoindex.html");
    // autoindexfile << autoindex ; 

}