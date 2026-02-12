#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <system_error>
#include <fcntl.h>
#include <cstring>

#include "event_loop.hpp"
#include "channel.hpp"

using moshi::EventLoop;
using moshi::Channel;

EventLoop::EventLoop() : default_thread_id_(std::this_thread::get_id()), flag_{false} {
    epoll_fd_ = epoll_create1(0);

    // 唤醒fd，用于及时触发epoll_wait，将其改造
    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(wakeup_fd_ < 0) {
       throw std::system_error(errno, std::generic_category(), "eventfd() failed");
    }

    if (epoll_fd_ < 0) {
        close(wakeup_fd_);
        throw std::system_error(errno, std::generic_category(), "epoll_create1() failed");
    }
    events_.resize(2048);  // 预分配事件数组

    epoll_event ev;
    ev.events = EPOLLIN;
    if(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev)) {
        close(wakeup_fd_);
        throw std::system_error(errno, std::generic_category(), "epoll_ctl() add wakeup_fd_ failed");
    }
}

EventLoop::~EventLoop() {
    stop();
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
    if(wakeup_fd_ >= 0) {
        close(wakeup_fd_);
    }
}

bool EventLoop::add_channel(int fd, std::shared_ptr<Channel> channel) {
    if(!verify_thread_id_()) {
        std::cout << "The EventLoop can only be added by its creator thread\n"; // 此处还应该添加一个日志
        return false;
    }

    if(epoll_ctl(epoll_fd_,
        EPOLL_CTL_ADD, fd,
        channel->get_epoll_events()) < 0) {
        std::cout << "epoll_ctl() add fd " << fd << " failed, errno: " << errno << ", error message: " << strerror(errno) << "\n";
        return false;
    }
    channel_[fd] = channel;
    return true;
}

bool EventLoop::remove_channel(int fd) {
    if(!verify_thread_id_()) {
        std::cout << "The EventLoop can only be removed by its creator thread\n"; // 此处还应该添加一个日志
        return false;
    }

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        return false;
    }
    channel_.erase(fd);
    return true;
}

bool EventLoop::start() {
    // 应该只能由绑定的线程执行
    if(verify_thread_id_() == false) {
        std::cout << "The EventLoop can only be started by its creator thread\n"; // 此处应该进行日志写入
        return false;
    }

    flag_ = true;
    while (flag_) {
        int n = epoll_wait(epoll_fd_, events_.data(), events_.size(), -1);
        // std::cout << "结束阻塞\n";
        if (n < 0)
            return false;

        for (int i = 0; i < n; ++i) {
            int fd = events_[i].data.fd;
            if(fd == wakeup_fd_) { // 如果这是唤醒线程，清除其中的数据就行
                std::cout << "尝试唤醒并停止EventLoop\n";
                size_t buf_size = 128;
                std::vector<uint64_t> temp_buf(buf_size);
                ssize_t cnt;
                while ((cnt = read(wakeup_fd_, &temp_buf, buf_size * 8)) > 0) {
                    std::cout << "EventLoop正在退出，成功读取唤醒数据，数据量为：" << cnt << "，数据量应当是8的倍数（可能唤醒多次）\n";

                    temp_buf.clear(); // 数据是没用的，可以直接清除数据
                    flag_ = false;
                }
                // 在很多的系统钟，EAGAIN和EWOULDBLOCK是相同的值
                // 但是由于历史遗留问题，还是需要同时检查这两个值
                // https://cloud.tencent.com/developer/ask/sof/105601782
                if (cnt == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cout << "退出错误\n";
                    throw std::system_error(errno, std::generic_category(),
                                        "EventLoop run in read() failed");
                }
            } // 新事件到来，通过Channel进行处理
            else {
                if(channel_.count(fd) != 0) { // 存在该事件，该事件未被删除 
                    channel_[fd]->handle_event();
                }
            }
        }
    }
    return true;
}


void EventLoop::wakeup_() {
    uint64_t flag = 1;
    ssize_t n = ::write(wakeup_fd_, &flag, sizeof(flag)); // 必须写入 8 字节
    if (n != sizeof(flag)) { // 错误处理：检查是否成功写入 8 字节
        // 处理写入错误，例如记录日志
        // LOG_ERROR << "EventLoop::wakeup() wrote " << n << " bytes instead of " << sizeof(one);
        std::cout << "EventLoop::wakeup_() wrote error\n";
    }
}