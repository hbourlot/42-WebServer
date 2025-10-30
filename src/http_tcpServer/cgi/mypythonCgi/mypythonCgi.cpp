#include "/home/mistery576/42-WebServer/inc/http_tcpServer/mypythonCgi.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>

PythonCGI::PythonCGI(http::Client *client, http::ClientManager *clientManager, std::string &filePath)
    : _client(client), _clientManager(clientManager), _filePath(filePath){
    _executePath = "/usr/bin/python3";
    
}

PythonCGI::~PythonCGI() {
	close(_inputPipe[0]);
	close(_inputPipe[1]);
	close(_outputPipe[0]);
	close(_outputPipe[1]);
}

static std::string to_string_98(size_t value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void PythonCGI::doDup(){
    if (pipe(_inputPipe) == -1) 
        std::cerr << "Invalide input" << std::endl;

    if (pipe(_outputPipe) == -1)
        std::cerr << "Invalid output" << std::endl;
}

void PythonCGI::executeChild(){
    // Redirect stdin to read end of inputPipe
    if (dup2(_inputPipe[0], STDIN_FILENO) == -1) 
        std::cerr << "error with input dup2" << std::endl;
    // Redirect stdout to write end of outputPipe
    if (dup2(_outputPipe[1], STDOUT_FILENO) == -1)
        std::cerr << "error with output dup2" << std::endl;
    
    // Close original fds after dup2
    close(_inputPipe[0]);
    close(_outputPipe[1]);

    // Builds argv
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(_executePath.c_str()));     // argv[0] == "python3"
    argv.push_back(const_cast<char*>(_filePath.c_str()));     // argv[1] == "/path/to/script.py"
    argv.push_back(NULL);

    // Build envp: typical CGI vars (add others as needed)
    std::vector<std::string> envStrings;
    envStrings.push_back("REQUEST_METHOD=POST");
    envStrings.push_back(std::string("CONTENT_LENGTH=") + to_string_98(_requestBody.size()));
    envStrings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
    envStrings.push_back("QUERY_STRING=");
    envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");

    std::vector<char*> envp;
    envp.reserve(envStrings.size() + 1);
    for (size_t i = 0; i < envStrings.size(); ++i)
        envp.push_back(const_cast<char*>(envStrings[i].c_str()));
    envp.push_back(NULL);

    // Execve: substitui o processo filho; se falhar, imprime erro (irá para parent's pipe)
    execve(_executePath.c_str(), argv.data(), envp.data());
    // se chegar aqui, execve falhou
    std::cerr << "excve failed" << std::endl;
    _exit(1);
}

void PythonCGI::execute() {
    const std::string scriptPath = "/home/mistery576/42-WebServer/var/www/cgi-bin/hello.py";
    const std::string requestBody = "name=alice&action=hello";
    
    doDup();
    // int inputPipe[2];  // parent writes -> child reads (stdin)
    // int outputPipe[2]; // child writes (stdout) -> parent reads

    // if (pipe(inputPipe) == -1) 
    //     std::cerr << "Invalide input" << std::endl;

    // if (pipe(outputPipe) == -1)
    //     std::cerr << "Invalid output" << std::endl;
    
    // Start fork
    pid_t pid = fork();
    
    if (pid == 0){
        executeChild();
    }

    close(_inputPipe[0]);   // parent doesn't read from inputPipe
    close(_outputPipe[1]);  // parent doesn't write to outputPipe

    // 1) Write the body for child STDIN request
    ssize_t written = 0;
    const char* buf = requestBody.c_str();
    size_t toWrite = requestBody.size();
    while (toWrite > 0) {
        ssize_t w = write(_inputPipe[1], buf + written, toWrite);
        if (w == -1) {
            if (errno == EINTR) continue;
            perror("write to child stdin");
            break;
        }
        written += w;
        toWrite -= w;
    }

    // Close the write pipe to signal the child EOF
    close(_inputPipe[1]);

    // 2) Reads everything that shild wrote
    char readBuf[4096];
    ssize_t n;
    std::string childOutput;
    while ((n = read(_outputPipe[0], readBuf, sizeof(readBuf))) > 0) {
        childOutput.append(readBuf, n);
    }
    if (n == -1) {
        perror("read from child stdout");
    }
    close(_outputPipe[0]);

    // 3) Waints for child process to finish
    int status = 0;
    waitpid(pid, &status, 0);

    std::cout << "Sai do filho" << std::endl;
    // 4) Show the CGI output, to send to the client
    std::cout << "=== CGI raw output START ===\n";
    std::cout << childOutput << std::endl;
    std::cout << "=== CGI raw output END ===\n";

    if (WIFEXITED(status)) {
        std::cout << "Child exited with status: " << WEXITSTATUS(status) << "\n";
    } else if (WIFSIGNALED(status)) {
        std::cout << "Child killed by signal: " << WTERMSIG(status) << "\n";
    }
}

// Booleans

bool PythonCGI::isRunning(){
    return true;
}

bool PythonCGI::isFinished(){
    return true;
}

bool PythonCGI::hasError(){
    return true;
}

// Getters

// Python executer
std::string PythonCGI::getPythonExecuter(){
    return _executePath;
}

// Script to execute
std::string PythonCGI::getFilePath() const{
    return _filePath;
}

std::string PythonCGI::getOutput(){
    return _requestBody;
}

void PythonCGI::sendResponse(){
    return ;
}
