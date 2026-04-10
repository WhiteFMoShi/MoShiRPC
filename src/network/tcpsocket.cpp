#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <system_error>

#include "network/tcpsocket.hpp"

using moshi::TcpSocket;

TcpSocket::TcpSocket(unsigned int loop) : loop_(loop) {
    Retry();
}

TcpSocket::~TcpSocket() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

int TcpSocket::Listen(const std::string& ip, uint16_t port, int backlog) noexcept {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }

    if (::listen(fd_, backlog) < 0) {
        return -1;
    }
    return 0;
}

int TcpSocket::Accept(std::string& client_ip, uint16_t& client_port) noexcept {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = ::accept(fd_, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        return -1;
    }

    client_ip = inet_ntoa(client_addr.sin_addr);
    client_port = ntohs(client_addr.sin_port);
    return client_fd;
}

int TcpSocket::Connect(const std::string& ip, uint16_t port) noexcept {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    return ::connect(fd_, (sockaddr*)&addr, sizeof(addr));
}

ssize_t TcpSocket::Send(int fd, const void* data, size_t len) noexcept {
    return ::send(fd, data, len, 0);
}

ssize_t TcpSocket::Recv(int fd, void* buf, size_t len) noexcept {
    return ::recv(fd, buf, len, 0);
}

void TcpSocket::Close(int fd) noexcept {
    ::close(fd);
}

void TcpSocket::Retry() noexcept {
    unsigned int temp = 0;
    while (fd_ < 0 && temp < loop_) {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        temp++;
    }
}

bool TcpSocket::IsAliviable() const noexcept {
    return GetSockfd() >= 0;
}
