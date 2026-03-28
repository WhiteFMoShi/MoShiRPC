#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

#include "event_loop.hpp"

using moshi::EventLoop;

EventLoop::EventLoop() : default_thread_id_(std::this_thread::get_id()) {
    epoll_fd_ = ::epoll_create1(0);
    if (epoll_fd_ < 0) {
        return;
    }

    // 唤醒fd：用于从其它线程 stop() 时打断 epoll_wait
    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) {
        ::close(epoll_fd_);
        return;
    }

    events_.resize(2048);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = wakeup_fd_;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev) < 0) {
        ::close(wakeup_fd_);
        ::close(epoll_fd_);
    }
}

EventLoop::~EventLoop() {
    stop();

    if (wakeup_fd_ >= 0) {
        ::close(wakeup_fd_);
        wakeup_fd_ = -1;
    }
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

bool EventLoop::add_fd(int fd, std::shared_ptr<moshi::Channel> channel) {
    if (!verify_thread_id_()) {
        std::cout << "The EventLoop can only be added by its creator thread\n";
        return false;
    }
    if (!channel) {
        return false;
    }

    channel->set_fd(fd);

    uint32_t listen_events = channel->get_focus_events();
    if (listen_events == 0) {
        std::cout << "add_fd failed: channel focus events is 0\n";
        return false;
    }

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = listen_events;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        std::cout << "epoll_ctl() add fd " << fd << " failed, errno: " << errno
                  << ", error message: " << ::strerror(errno) << "\n";
        return false;
    }

    channel_[fd] = std::move(channel);
    return true;
}

bool EventLoop::reset_channel(int fd, std::shared_ptr<moshi::Channel> channel) {
    if (!verify_thread_id_()) {
        std::cout << "The EventLoop can only be reset by its creator thread\n";
        return false;
    }
    if (!channel) {
        return false;
    }

    channel->set_fd(fd);

    uint32_t listen_events = channel->get_focus_events();
    if (listen_events == 0) {
        std::cout << "reset_channel failed: channel focus events is 0\n";
        return false;
    }

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = listen_events;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        std::cout << "epoll_ctl() mod fd " << fd << " failed, errno: " << errno
                  << ", error message: " << ::strerror(errno) << "\n";
        return false;
    }

    channel_[fd] = std::move(channel);
    return true;
}

bool EventLoop::remove_fd(int fd) {
    if (!verify_thread_id_()) {
        std::cout << "The EventLoop can only be removed by its creator thread\n";
        return false;
    }

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        return false;
    }
    channel_.erase(fd);
    return true;
}

bool EventLoop::start() {
    if (!verify_thread_id_()) {
        std::cout << "The EventLoop can only be started by its creator thread\n";
        return false;
    }

    bool expected = false;
    if (!flag_.compare_exchange_strong(expected, true)) {
        // 已经在运行
        return false;
    }

    return eventloop_run_();
}

bool EventLoop::stop() {
    if(!flag_.load())
        return true;

    flag_.store(false);
    wakeup_();
    return true;
}

// 清理wakeup_中的内容，避免重复触发wakeup_
void EventLoop::drain_wakeup_fd_() {
    uint64_t value = 0;
    while (true) {
        const ssize_t n = ::read(wakeup_fd_, &value, sizeof(value));
        if (n == static_cast<ssize_t>(sizeof(value))) {
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        if (n < 0) {
            // 这里不抛异常，避免 stop 路径把线程打崩；直接退出循环即可
            break;
        }
        break;
    }
}

void EventLoop::wakeup_() {
    const uint64_t one = 1;
    const ssize_t n = ::write(wakeup_fd_, &one, sizeof(one));
    if (n != static_cast<ssize_t>(sizeof(one))) {
        std::cout << "EventLoop::wakeup_() wrote error\n";
    }
}

bool EventLoop::eventloop_run_() {
    while (flag_.load()) {
        int n = ::epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), -1);
        if (n < 0) {
            if (errno == EINTR) { // 系统是由于中断而导致的错误，不是真正的错误，重新尝试就可以了
                continue;
            }

            // 不是EINTR，可能是遇到了严重错误
            flag_.store(false);
            return false;
        }

        // 事件处理
        for (int i = 0; i < n; ++i) {
            const int fd = events_[i].data.fd;
            const uint32_t revent = events_[i].events;

            if (fd == wakeup_fd_) {
                drain_wakeup_fd_();
                continue;
            }

            auto it = channel_.find(fd);
            if (it == channel_.end() || !it->second) {
                continue;
            }

            it->second->handle_event(revent);
        }
    }
    return true;
}