#pragma once

#include <functional>
#include <vector>
#include <sys/epoll.h>

class EventLoop {
public:
    using EventCallback = std::function<void(int fd, uint32_t events)>;

    EventLoop();
    ~EventLoop();

    // 添加/修改事件监听
    void add_event(int fd, uint32_t events, EventCallback callback);
    void remove_event(int fd);

    // 运行事件循环（阻塞）
    void run();

private:
    int epoll_fd_;
    std::vector<epoll_event> events_; // 用于存储epoll_wait返回的就绪事件
    std::unordered_map<int, EventCallback> callbacks_;
};