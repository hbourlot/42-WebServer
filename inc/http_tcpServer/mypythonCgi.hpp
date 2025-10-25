#ifndef MYPYTHONCGI_HPP
#define MYPYTHONCGI_HPP

#include <string>
#include <map>

class CGI {
private:
    std::string _scriptPath;
    std::string _method;
    std::string _queryString;
    std::string _body;
    std::map<std::string, std::string> _env;

    void setupEnv();

public:
    CGI(const std::string &scriptPath,
        const std::string &method,
        const std::string &queryString = "",
        const std::string &body = "");

    std::string run();
};

#endif
