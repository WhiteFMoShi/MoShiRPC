#include <unistd.h>
#include <sys/epoll.h>
#include <system_error>
// #include <stdexcept>

#include "event_loop.hpp"

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        throw std::system_error(errno, std::generic_category(), "epoll_create1() failed");
    }
    events_.resize(1024);  // 预分配事件数组
}

EventLoop::~EventLoop() {
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
}

void EventLoop::add_event(int fd, uint32_t events, EventCallback callback) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::system_error(errno, std::generic_category(), "epoll_ctl(ADD) failed");
    }
    // 保存需要执行的回调函数
    callbacks_[fd] = callback;
}

void EventLoop::remove_event(int fd) {
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        throw std::system_error(errno, std::generic_category(), "epoll_ctl(DEL) failed");
    }
    callbacks_.erase(fd);
}

void EventLoop::run() {
    while (true) {
        int n = epoll_wait(epoll_fd_, events_.data(), events_.size(), -1);
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "epoll_wait() failed");
        }

        for (int i = 0; i < n; ++i) {
            int fd = events_[i].data.fd;
            uint32_t events = events_[i].events;
            if (callbacks_.count(fd)) {
                callbacks_[fd](fd, events);
            }
        }
    }
}