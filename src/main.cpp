#include <iostream>
#include "server.hpp"

int main()
{
    HttpServer server;
    server.initServer();
    server.acceptConnections();

    return 0;
}