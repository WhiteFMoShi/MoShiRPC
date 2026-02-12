#pragma once

#include <string>
namespace moshi {

struct NetworkConfig {
    std::string ip = "0.0.0.0";  // 监听 IP
    uint16_t port = 8080;        // 监听端口
    int backlog = 128;           // 最大等待连接数
    int timeout_ms = 5000;       // 超时时间（毫秒）
};

} // namespace moshi