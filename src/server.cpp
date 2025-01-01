#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "server.hpp"

#define PORT 8080

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
}

void HttpServer::acceptConnections()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Accept incoming connections
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) > 0)
    {
        std::cout << "New connection accepted!" << std::endl;
        close(new_socket); // Close the connection after handling it
    }

    // Error handling
    if (new_socket < 0)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
}

HttpServer::~HttpServer()
{
    if (server_fd != -1)
        close(server_fd);
    if (new_socket != -1)
        close(new_socket);
}