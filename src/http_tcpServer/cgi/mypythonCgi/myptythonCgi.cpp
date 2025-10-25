#include "mypythonCgi.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

CGI::CGI(const std::string &scriptPath,
         const std::string &method,
         const std::string &queryString,
         const std::string &body)
    : _scriptPath(scriptPath), _method(method),
      _queryString(queryString), _body(body) {
    setupEnv();
}

void CGI::setupEnv() {
    _env["REQUEST_METHOD"] = _method;
    _env["QUERY_STRING"]   = _queryString;
    _env["SCRIPT_NAME"]    = _scriptPath;
    _env["SERVER_PROTOCOL"]= "HTTP/1.1";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";

    if (_method == "POST") {
        _env["CONTENT_LENGTH"] = std::to_string(_body.size());
        _env["CONTENT_TYPE"]   = "application/x-www-form-urlencoded"; // application path
    }
}

std::string CGI::run() {
    int inPipe[2];   // Send information
    int outPipe[2];  // Read information
    if (pipe(inPipe) < 0 || pipe(outPipe) < 0) {
        perror("pipe");
        return "Status: 500 Internal Server Error\r\n\r\n";
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "Status: 500 Internal Server Error\r\n\r\n";
    }

    if (pid == 0) {
        // FILHO
        dup2(inPipe[0], STDIN_FILENO);   // body -> stdin
        dup2(outPipe[1], STDOUT_FILENO); // saída CGI -> stdout

        close(inPipe[1]);
        close(outPipe[0]);

        // Build envp for execve
        std::vector<char*> envp;
        for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it) {
            std::string entry = it->first + "=" + it->second;
            char *cstr = strdup(entry.c_str());
            envp.push_back(cstr);
        }
        envp.push_back(NULL);

        char *argv[] = { strdup(_scriptPath.c_str()), NULL };
        execve(_scriptPath.c_str(), argv, envp.data());
        perror("execve");
        exit(1);
    }
    else {
        // Main process
        close(inPipe[0]);
        close(outPipe[1]);

        // Check post method
        if (_method == "POST" && !_body.empty()) {
            write(inPipe[1], _body.c_str(), _body.size());
        }
        close(inPipe[1]);

        // Read Cgi exit
        std::string output;
        char buffer[4096];
        ssize_t n;
        while ((n = read(outPipe[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, n);
        }
        close(outPipe[0]);

        // Wait for child process
        int status;
        waitpid(pid, &status, 0);

        return output;
    }
}