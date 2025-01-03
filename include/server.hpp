#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>

struct HttpRequest
{
    std::string method;
    std::string uri;
    std::string body;
    std::map<std::string, std::string> headers;
};

class HttpServer
{
public:
    HttpServer();
    void initServer();
    void acceptConnections();
    static void stopServer(int signum);
    void cleanup();
    void handleRequest(int client_socket);
    HttpRequest parseRequest(const std::string &request);
    ~HttpServer();

private:
    int server_fd, new_socket;
};

#endif
