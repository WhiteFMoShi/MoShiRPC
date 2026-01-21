#pragma once

#include "event_loop.hpp"
#include <cstdint>
#include <functional>
#include <memory>

// #include "event_loop.hpp" // 此处由于不需要使用EventLoop中的函数
                            // 因此不需要包含头文件，只需要向前声明
class EventLoop;

/**
 * @brief 用于处理每个文件描述符的事件
 * 
 */
class Channel {
public:
    // 事件类型声明（加上static表示这个变量不是成员而是共有的，可以减少内存占用）
    static const uint32_t NONE = 0b0;
    static const uint32_t READABLE = 0b1;
    static const uint32_t WRITABLE = 0b10;
    static const uint32_t ERROR = 0b100;

    // 回调函数声明
    using ReadCallback = std::function<void()>;
    using WriteCallback = std::function<void()>;
    using ErrorCallback = std::function<void()>;

public:
    Channel() = delete;
    Channel(std::shared_ptr<EventLoop> loop, int fd, uint32_t enents);
    Channel(std::shared_ptr<EventLoop> loop, int fd); // 绑定一个EventLoop
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    ~Channel();

    bool update_events(uint32_t events) { events_ = events; return true; }

    bool enable_read();
    bool enable_write();
    bool enable_error();

    void set_read_callback(ReadCallback cb) {
        readCallback_ = std::move(cb);
    }
    void set_write_callback(WriteCallback cb) {
        writeCallback_ = std::move(cb);
    }
    void set_error_callback(ErrorCallback cb) {
        errorCallback_ = std::move(cb);
    }

    bool is_readable() const { return (events_ & READABLE) != NONE; }
    bool is_writable() const { return (events_ & WRITABLE) != NONE; }
    bool is_error()    const { return (events_ & ERROR)    != NONE; }

private:
    std::weak_ptr<EventLoop> loop_; // 所属的EventLoop（依赖注入模式）
    int fd_; // 关注的文件描述符
    uint32_t events_; // 关注的事件类型

    ReadCallback readCallback_;
    WriteCallback writeCallback_;
    ErrorCallback errorCallback_;
};