#include "server.hpp"

std::string handleGetRoot(const HttpRequest &request)
{
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: 17\r\n"
           "\r\n"
           "Hello from GET!";
}

std::string handlePostRoot(const HttpRequest &request)
{
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: 18\r\n"
           "\r\n"
           "Hello from POST!";
}

int main()
{
    HttpServer server;
    server.registerRoute("GET", "/", handleGetRoot);
    server.registerRoute("POST", "/", handlePostRoot);
    server.initServer();
    server.acceptConnections();
    return 0;
}
