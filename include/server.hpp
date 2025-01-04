#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <functional>

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
    ~HttpServer();
    void initServer();
    void acceptConnections();
    static void stopServer(int signum);
    void cleanup();
    void handleRequest(int client_socket);
    HttpRequest parseRequest(const std::string &request);
    using HandlerFunction = std::function<std::string(const HttpRequest &)>;
    void registerRoute(const std::string &method, const std::string &uri, HandlerFunction handler);
    std::string generateResponse(const HttpRequest &request);

private:
    int server_fd, new_socket;
    std::map<std::string, HandlerFunction> routes; // Map of method:URI -> handler
};

#endif
