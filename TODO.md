# Webserver Project - TODO.md

## PHASE 1: Project Setup and Research

-   [x] Read and understand project requirements (subject PDF)
-   [...] Research HTTP/1.1 protocol (methods, headers, status codes)
-   [...] Study socket programming (bind, listen, accept, recv, send)
-   [x] Set up Makefile and project folder structure
-   [x] Initialize Git and .gitignore

## PHASE 2: Core Structure

-   [x] Implement basic TCP socket server
-   [ ] Add support for multiple clients using select() or poll()
-   [ ] Parse incoming HTTP requests (method, path, headers, body)
-   [ ] Build and send proper HTTP responses
-   [x] Handle status codes (200, 404, 500, etc.)

## PHASE 3: Configuration and Routing

-   [ ] Design and parse server config file (.conf)
-   [ ] Define server and location blocks (ports, root paths, error pages)
-   [ ] Implement virtual hosting (based on hostNames or ports)
-   [ ] Implement routing based on configuration

## PHASE 4: Method Handling

-   [x] Handle GET requests (serve static files)
-   [x] Handle POST requests (receive and store body content)
-   [ ] Handle DELETE requests (remove allowed files)
-   [ ] Support Content-Type and Content-Length headers
-   [ ] Implement CGI support (execute scripts, return output)

## PHASE 5: Extra Features and Testing

-   [ ] Serve custom error pages
-   [ ] Implement request timeouts
-   [ ] Prevent directory traversal (sanitize paths)
-   [ ] Perform stress and concurrency testing (curl, ab, siege)

## PHASE 6: Bonus Features (Optional)

-   [ ] Add HTTPS support using OpenSSL
-   [ ] Generate auto-index directory listings
-   [ ] Implement cookie-based sessions
-   [ ] Add request and error logging

## PHASE 7: Final Touches

-   [ ] Refactor code (OOP, classes, const correctness)
-   [ ] Comment and document all modules
-   [ ] Write README.md with build/run instructions
-   [ ] Final tests with edge cases and malformed requests
