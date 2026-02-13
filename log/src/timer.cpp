#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "timer.hpp"

AdvancedConditionalTimer::~AdvancedConditionalTimer() {
    stop();
}

void AdvancedConditionalTimer::start_min(const int& min, OnTimeCallback callback) {
    if(running_)
        throw std::logic_error("AdvancedConditionalTimer::start_min: Timer already running!!!");

    running_ = true;

    worker_ = std::thread([this, min, callback]() {
        std::unique_lock<std::mutex> locker(mtx_);
        while(running_) {
            // 睡眠指定时间
            std::cv_status status = cv_.wait_for(locker, std::chrono::minutes(min));

            // 若是是被唤醒的，说明有文件写入，重置定时器
            // 若是超时，结束定时器
            if(status == std::cv_status::timeout) {
                running_ = false;
                callback();
                break;
            }
        }
    });
}

void AdvancedConditionalTimer::reset_timer() {
    cv_.notify_all();
}

void AdvancedConditionalTimer::stop() {
    running_ = false;

    cv_.notify_all();
    if(worker_.joinable())
        worker_.join();
}