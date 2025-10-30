#ifndef MYPYTHONCGI_HPP
#define MYPYTHONCGI_HPP

#include "../Client/Client.hpp"
#include "ICgi.hpp"
#include <string>
#include <map>

class PythonCGI : public http::ICgi {
private:
    http::Client        *_client;           // Client to execute CGI
    http::ClientManager *_clientManager;    // Client manager
    
    std::string         _filePath;          // Script to execute
    std::string         _executePath;       // Path to python execute
    std::string         _requestBody;       // Body of a HTTP request
    
    int                 _inputPipe[2];
    int                 _outputPipe[2];
    
public:

    PythonCGI(http::Client *client, http::ClientManager *manager, std::string &filePath);
    ~PythonCGI();
    void doDup();
    void executeChild();
    void execute();

    bool isRunning ();
    bool isFinished();
    bool hasError();

    // Read output from the CGI process
    // virtual void readOutput() = 0;

    // Send the CGI response to the client (write to socket)
    void sendResponse();

    // Get the python script executer
    std::string getPythonExecuter();

    // Get the file path of the CGI script/file
    std::string getFilePath() const;

    // Get the output content (header + body)
    std::string getOutput();

};

#endif
