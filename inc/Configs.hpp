#pragma once
#include <vector>
#include <string>

struct Route{
    std::string                 root;  
};

struct Configs{
    int                         port;
    std::vector<std::string>    methods;
    std::vector<Route>          routes;
};