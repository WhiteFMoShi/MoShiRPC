#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

using OnTimeCallback = std::function<void()>;

class AdvancedConditionalTimer {
public:
    AdvancedConditionalTimer() = default;
    AdvancedConditionalTimer(AdvancedConditionalTimer&) = delete;
    AdvancedConditionalTimer(AdvancedConditionalTimer&&) = delete;
    AdvancedConditionalTimer operator=(AdvancedConditionalTimer&&) = delete;
    AdvancedConditionalTimer operator=(const AdvancedConditionalTimer&) = delete;

    ~AdvancedConditionalTimer();

    /**
     * @brief 设定一个min分钟的定时器，若是定时器超时将会触发callback，若是使用reset_timer()将会重设倒计时。
     * 
     * @param min 启动一个以分钟为单位的定时器
     * @param callback 超时执行的回调函数
     */
    void start_min(const int& min, OnTimeCallback callback);

    /**
     * @brief 重设定时器，此时若是定时器处于running状态，会立马被唤醒并重新进入倒计时；
     *          若是stop()已经被调用再reset_timer()，将不会有任何行为
     */
    void reset_timer();

    /**
     * @brief 停止定时器，在停止后需要使用start_*重新设定定时器
     * 
     */
    void stop();
private:
    OnTimeCallback callback_;
    std::atomic<bool> running_ = false;

    std::mutex mtx_;
    std::condition_variable cv_;

    std::thread worker_;
};