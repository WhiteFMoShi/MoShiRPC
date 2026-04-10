#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <sys/epoll.h>

#include "channel.hpp"

namespace moshi {

/**
 * @brief 事件触发器
 *        会对线程进行绑定
 *        以免在执行的时候，由于多线程异常对EventLoop进行修改而出现异常
 */
class EventLoop {
public:
    /**
     * @brief 创建一个epollfd，若是创建失败会抛出异常
     *
     * @throw std::system_error 系统的文件描述符资源不足以继续分配
     */
    EventLoop();
    ~EventLoop();

    /**
     * @brief 添加fd到epoll监听，并绑定一个Channel做事件分发
     *        不允许在多个线程中调用该函数，该函数的调用只允许在创建EventLoop对象的线程中使用
     */
    bool AddFd(int fd, std::shared_ptr<Channel> channel);

    /**
     * @brief 更新已监听fd绑定的Channel及其关注事件（epoll_ctl MOD）
     */
    bool ResetChannel(int fd, std::shared_ptr<Channel> channel);

    /**
     * @brief 不再监听fd上到来的事件，专注于删除监听事件，不会关闭文件描述符
     */
    bool RemoveFd(int fd);

    /**
     * @brief 启动EventLoop（只能由创建线程调用）
     */
    bool Start();

    /**
     * @brief 请求停止事件循环（线程安全）
     */
    bool Stop();

    bool GetRunStatus() const { return running_.load(); }

private:
    bool VerifyThreadId_() const { return std::this_thread::get_id() == default_thread_id_; }

    void Wakeup_();
    void DrainWakeupFd_();
    bool EventloopRun_();

private:
    std::atomic<bool> running_{false};
    std::thread::id default_thread_id_;

    int epoll_fd_{-1};
    int wakeup_fd_{-1};

    std::vector<epoll_event> events_;
    std::map<int, std::shared_ptr<Channel>> channel_; // <fd, Channel>
};

} // namespace moshi
