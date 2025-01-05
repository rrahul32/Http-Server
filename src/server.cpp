#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <atomic>
#include <chrono>
#include <sstream>

#include "server.hpp"

#define PORT 8080

// Global flag to indicate when to shut down
std::atomic<bool> is_shutdown(false);

// Initialize the server
HttpServer::HttpServer() : server_fd(-1), new_socket(-1) {}

void HttpServer::initServer()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    address.sin_port = htons(PORT);       // Port number to listen on (convert to network byte order)

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) // typecast to sockaddr pointer from sockaddr_in for compatibility
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Set up signal handler for cleanup
    signal(SIGINT, &HttpServer::stopServer);
}

void HttpServer::acceptConnections()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Accept incoming connections
    while (!is_shutdown)
    {
        // Use select or poll to check for new connections or signals
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        // Wait for an incoming connection or signal interruption
        int activity = select(server_fd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0)
        {
            if (is_shutdown)
                break; // If shutdown is requested, exit the loop
            perror("Select failed");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            // Accept new connection
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

            if (new_socket < 0)
            {
                if (is_shutdown)
                    break; // If shutdown flag is set, stop accepting
                perror("Accept failed");
                continue;
            }

            std::cout << "New connection accepted!" << std::endl;

            handleRequest(new_socket);
        }
    }
}

void HttpServer::stopServer(int signum)
{
    std::cout << "\nServer stopped." << std::endl;

    is_shutdown = true;
}

void HttpServer::cleanup()
{
    // Close the server socket
    if (server_fd != -1)
        close(server_fd);
    if (new_socket != -1)
        close(new_socket);
}

void HttpServer::handleRequest(int client_socket)
{
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, 1024);

    if (bytes_read < 0)
    {
        perror("Error reading from socket");
        close(client_socket);
        return;
    }

    std::string rawRequest(buffer, bytes_read);
    HttpRequest request = parseRequest(rawRequest);

    std::string response = generateResponse(request);

    int bytes_sent = send(client_socket, response.c_str(), response.size(), 0);
    if (bytes_sent < 0)
    {
        perror("Error sending response");
    }
    close(client_socket);
}

HttpServer::~HttpServer()
{
    cleanup();
}

HttpRequest HttpServer::parseRequest(const std::string &request)
{
    HttpRequest httpRequest;

    // Parse the request
    std::istringstream stream(request);
    std::string line;

    // parse the method and uri
    if (std::getline(stream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> httpRequest.method >> httpRequest.uri;

        // parse the query params
        int queryPos = httpRequest.uri.find('?');
        if (queryPos != std::string::npos)
        {
            std::string queryString = httpRequest.uri.substr(queryPos + 1);
            httpRequest.uri = httpRequest.uri.substr(0, queryPos);
            httpRequest.queryParams = getQueryParams(queryString);
        }
    }

    // parse the headers
    while (std::getline(stream, line) && line != "\r\n")
    {
        auto delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos)
        {
            httpRequest.headers[line.substr(0, delimiterPos)] = line.substr(delimiterPos + 2);
        }
    }

    // parse rest as the body
    std::ostringstream bodyStream;
    bodyStream << stream.rdbuf();
    httpRequest.body = bodyStream.str();

    // debug
    std::cout << "Method: " << httpRequest.method << std::endl;
    std::cout << "URI: " << httpRequest.uri << std::endl;
    std::cout << "Query Params: " << std::endl;
    for (const auto &param : httpRequest.queryParams)
    {
        std::cout << param.first << ": " << param.second << std::endl;
    }
    std::cout << "Headers: " << std::endl;
    for (const auto &header : httpRequest.headers)
    {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << "Body: " << httpRequest.body << std::endl;

    return httpRequest;
}

void HttpServer::registerRoute(const std::string &method, const std::string &uri, HandlerFunction handler)
{
    std::string key = method + ":" + uri;
    routes[key] = handler;
}

std::string HttpServer::generateResponse(const HttpRequest &request)
{
    std::string key = request.method + ":" + request.uri;
    if (routes.find(key) != routes.end())
    {
        // Call the handler function for the route
        return routes[key](request);
    }

    // Default response for unhandled routes
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: 13\r\n"
           "\r\n"
           "404 Not Found";
}

std::map<std::string, std::string> HttpServer::getQueryParams(const std::string &query)
{
    std::map<std::string, std::string> queryParams;
    std::istringstream queryStream(query);
    std::string param;
    while (std::getline(queryStream, param, '&'))
    {
        auto delimiterPos = param.find('=');
        if (delimiterPos != std::string::npos)
        {
            queryParams[param.substr(0, delimiterPos)] = param.substr(delimiterPos + 1);
        }
    }

    return queryParams;
}
