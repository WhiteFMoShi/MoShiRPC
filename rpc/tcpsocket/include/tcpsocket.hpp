#pragma once

#include <sys/socket.h>
#include <string>

class TcpSocket {
public:
    bool connect(std::string ipv4_addr, uint port);
    bool close(int fd);
    bool listen();

    bool send_msg();
    bool receive_msg(uint fd);
private:
    int socket_fd_; // listening port
};