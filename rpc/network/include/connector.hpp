#pragma once

#include <memory>

#include "tcpsocket.hpp"
#include "event_loop.hpp"
#include "base_buffer.hpp"

/**
 * @brief 提供网络连接的高级抽象
 * 
 */
class Connector {
public:
    Connector();
    ~Connector();
private:
    std::unique_ptr<moshi::BaseBuffer> buffer_; // 用于存储接收到的数据,使用继承机制进行抽象
    std::unique_ptr<moshi::EventLoop> loop_;
    moshi::TcpSocket socket_; // 用于实际的网络通信
};