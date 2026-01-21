#pragma once

#include <functional>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <thread>
#include <sys/epoll.h>
#include <atomic>
#include <unordered_map>

#include "channel.hpp"

class Channel;

/**
 * @brief 事件触发器
 *        会对线程进行绑定
 *        以免在执行的时候，由于多线程异常对EventLoop进行修改而出现异常
 */
class EventLoop {
public:
    using EventCallback = std::function<void(int fd, uint32_t events)>;

    /**
     * @brief 创建一个epollfd，若是创建失败会抛出异常
     * 
     * @throw std::system_error 系统的文件描述符资源不足以继续分配
     */
    EventLoop();
    ~EventLoop();

    /**
     * @brief 向添加的文件描述符增加监听的事件，同时绑定一个callback
     *        不允许在多个线程中调用该函数，该函数的调用只允许在创建EventLoop对象的线程中使用
     * 
     * @param fd 
     * @param events 
     * @param callback 
     * @return true 
     * @return false 
     */
    bool add_event(int fd, uint32_t events, EventCallback callback);
    /**
     * @brief 不再监听fd上到来的事件，专注于删除监听事件，不会关闭文件描述符
     * 
     * @param fd 
     * @return true 
     * @return false 
     */
    bool remove_event(int fd);

    /**
     * @brief 启动EventLoop
     * 
     * @exception std::runtime_error 由于wakeup_fd_被设置为非阻塞式的，若是在运行时，read出现异常便会抛出 (这里出现的情况太多了)
     * @return true 
     * @return false 因为异常原因，EventLoop启动失败
     */
    bool run();

    // 停止事件循环
    bool stop();

    /**
     * @brief 获取当前的运行状态
     * 
     * @return true 正在运行中
     * @return false 处于停止状态
     */
    bool run_status() const;

private:
    /**
     * @brief 验证线程绑定，防止误操作
     * 
     * @return true 
     * @return false 
     */
    bool verify_thread_id_() const;

    void wakeup_();
private:
    int epoll_fd_;
    int wakeup_fd_; // 非阻塞IO
    
    std::atomic<int> flag_;
    std::thread::id thread_id_; // 绑定线程ID

    std::vector<epoll_event> events_; // 用于存储epoll_wait返回的就绪事件
    std::unordered_map<int, std::shared_ptr<Channel>> callbacks_; // 文件描述符和对应的事件触发器
};