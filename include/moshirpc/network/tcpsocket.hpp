#pragma once

#include <string>

namespace moshi {
/**
 * @brief 提供socket的易用、基础封装
 */
class TcpSocket {
public:
    TcpSocket(unsigned int retry_num = 100);
    ~TcpSocket();

    // 绑定并监听端口
    // 其中的backlog是接收队列的最大长度
    int Listen(const std::string& ip, uint16_t port, int backlog = 128) noexcept;
    
    // 接受新连接（返回新 Socket 的文件描述符）
    int Accept(std::string& client_ip, uint16_t& client_port) noexcept;

    // 连接远程服务器
    int Connect(const std::string& ip, uint16_t port) noexcept;

    // 发送数据（返回实际发送的字节数）
    ssize_t Send(int fd, const void* data, size_t len) noexcept;

    // 接收数据（返回实际接收的字节数）
    ssize_t Recv(int fd, void* buf, size_t len) noexcept;

    // 关闭 Socket
    void Close(int fd) noexcept;

    int GetSockfd() const noexcept { return fd_; }

    void Retry() noexcept;

    bool IsAliviable() const noexcept;

private:
    int fd_ = -1;  // 监听 Socket 的文件描述符
    unsigned int loop_ = 100;
};

} // namespace moshi