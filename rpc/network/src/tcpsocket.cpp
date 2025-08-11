#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <system_error>

#include "tcpsocket.hpp"

TcpSocket::TcpSocket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        throw std::system_error(errno, std::generic_category(), "socket() failed");
    }
}

TcpSocket::~TcpSocket() {
    if (listen_fd_ >= 0) {
        close(listen_fd_);
    }
}

void TcpSocket::listen(const std::string& ip, uint16_t port, int backlog) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "bind() failed");
    }

    if (::listen(listen_fd_, backlog) < 0) {
        throw std::system_error(errno, std::generic_category(), "listen() failed");
    }
}

int TcpSocket::accept(std::string& client_ip, uint16_t& client_port) {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = ::accept(listen_fd_, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "accept() failed");
    }

    client_ip = inet_ntoa(client_addr.sin_addr);
    client_port = ntohs(client_addr.sin_port);
    return client_fd;
}

void TcpSocket::connect(const std::string& ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "connect() failed");
    }
}

ssize_t TcpSocket::send(int fd, const void* data, size_t len) {
    return ::send(fd, data, len, 0);
}

ssize_t TcpSocket::recv(int fd, void* buf, size_t len) {
    return ::recv(fd, buf, len, 0);
}

void TcpSocket::close(int fd) {
    ::close(fd);
}