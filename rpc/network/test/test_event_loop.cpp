#include "event_loop.hpp"
#include "tcpsocket.hpp"
#include <iostream>

int main() {
    EventLoop loop;
    TcpSocket server;
    server.listen("127.0.0.1", 8080);

    char buff[256];

    loop.add_event(server.get_sockfd(), EPOLLIN, [&](int fd, uint32_t events) {
        std::string client_ip;
        uint16_t client_port;
        int client_fd = server.accept(client_ip, client_port);
        std::cout << "New connection from " << client_ip << ":" << client_port << std::endl;
        server.recv(client_fd, buff, 256);

        printf("%s\n", buff);
    });

    loop.run();
    return 0;
}