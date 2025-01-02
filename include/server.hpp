#ifndef SERVER_HPP
#define SERVER_HPP

class HttpServer
{
public:
    HttpServer();
    void initServer();
    void acceptConnections();
    static void stopServer(int signum);
    void cleanup();
    void handleRequest(int client_socket);
    ~HttpServer();

private:
    int server_fd, new_socket;
};

#endif
