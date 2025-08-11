#include "tcpsocket.hpp"
#include <iostream>
#include <thread>

void server_thread() {
    TcpSocket server;
    server.listen("127.0.0.1", 8080);

    std::string client_ip;
    uint16_t client_port;
    int client_fd = server.accept(client_ip, client_port);

    char buf[1024];
    ssize_t len = server.recv(client_fd, buf, sizeof(buf));
    std::cout << "Server received: " << std::string(buf, len) << std::endl;

    server.close(client_fd);
}

int main() {
    std::thread t(server_thread);
    t.detach();

    TcpSocket client;
    client.connect("127.0.0.1", 8080);
    client.send(client.get_sockfd(), "Hello RPC", 9);

    return 0;
}