#pragma once

#include <string>

class TcpSocket {
public:
    TcpSocket();
    ~TcpSocket();

    // 绑定并监听端口
    // 其中的backlog是接收队列的最大长度
    void listen(const std::string& ip, uint16_t port, int backlog = 128);
    
    // 接受新连接（返回新 Socket 的文件描述符）
    int accept(std::string& client_ip, uint16_t& client_port);

    // 连接远程服务器
    void connect(const std::string& ip, uint16_t port);

    // 发送数据（返回实际发送的字节数）
    ssize_t send(int fd, const void* data, size_t len);

    // 接收数据（返回实际接收的字节数）
    ssize_t recv(int fd, void* buf, size_t len);

    // 关闭 Socket
    void close(int fd);

    int get_sockfd() const { return listen_fd_; }

private:
    int listen_fd_ = -1;  // 监听 Socket 的文件描述符
};