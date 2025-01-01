#ifndef SERVER_HPP
#define SERVER_HPP

class HttpServer
{
public:
    HttpServer();
    void initServer();
    void acceptConnections();
    ~HttpServer();

private:
    int server_fd, new_socket;
};

#endif
