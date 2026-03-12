e*This project has been created as part of the 42 curriculum by hbourlot, joralves, miafonso.*

# Webserv

## Description

Webserv is an implementation of an HTTP server in C++, developed as a part of the 42 curriculum.
The objective of this project is to understand how it works a low-level web page, HTTP protocol, connection management and CGI executions

The server can do:
* Accept TCP connections
* Execute CGI
* Play videos
* Manage multiple clients

This project focuses on performance and manages all clients requests without blocking.

---

## Instructions

### Compilation

Requirements:

* C++98
* make
* Linux / WSL

Compile the project:

```bash
make
```

Execute

```bash
./webserv <config_file>
```

Example:

```bash
./webserv config/default.conf
```

---

### Configuration

The server use a `.conf` file with the follow possibilities:

### Server configs

* host  ==> Default value: 127.0.0.1
* port  ==> Default value:
* server_name ==> Default value: Default
* client_max_body_size ==> Default value: 1024
* error_page ==> Default value: 404 ../../content/defaultError.html
* location 
* root (optional)
* index (optional)
* cgi_pass (optional)
* client_body_temp_path ==> Default value: ./tmp
* client_body_buffer_size ==> Default value: 1048576
* keepalive_timeout ==> Default value: 3
* auth_login_page (optional)

### Locations configs

* methods ==> Default value: GET
* root ==> Default value: var/www
* redirect (optional)
* cgi_extension (optional)
* cgi_path (optional)
* upload_enable ==> Default value: false
* upload_store (optional)
* autoindex (optional)
* index (optional)
* cgi_pass (optional)
* client_body_buffer_size ==> Default value: 1048576
* auth_login_page (optional)
* auth (optional)

Example:

```
server {
    listen 8080;
    host 127.0.0.1;
    server_name localhost;
    root ./www;

    location / {
        methods GET ;
    }
}
```

---

### Execution

Open the server using (./webserv <file.conf>)

Open a browser:

```
http://localhost:port
```

Or use curl:

```bash
curl http://localhost:port
```

Testr POST:

```bash
curl -X POST http://localhost:8080 -d "data=test"
```

---

## Features

* HTTP/1.1 support
* Multiple server blocks
* Non-blocking I/O
* Poll/select/epoll/kqueue
* CGI execution
* Error handling pages
* Request parsing
* Configuration file parsing
* Video player
* File upload

---

## Technical Choices

* C++98
* Arquitecture based on:

  * Socket manager
  * Request parser
  * Response builder
  * Configuration parser
  * CGI execution
  * I/O multiplexing to support multiple clients

---

## Resources

### Documentation

* RFC 7230 – Hypertext Transfer Protocol (HTTP/1.1)
* Beej's Guide to Network Programming
* Nginx configuration documentation
* Linux man pages:

  * socket
  * bind
  * listen
  * accept
  * poll / select / epoll
  * CGI behaviour

### Tutorials & Articles

* HTTP protocol explained
* Writing a web server from scratch
* TCP/IP networking fundamentals
* Parsing NGINX configuration
* Struct NGINX configuration

### AI Usage

We used AI tools to help us in some criterions:

* Understand how it works a NGINX
* CGI behaviour in a web server
* NGINX configuration parameters
* Suggestions to struct the code
* Helped to correct (README.md)

No code was copied directly from AI tools

---

## Authors

* hbourlot
* joralves
* miafonso

---