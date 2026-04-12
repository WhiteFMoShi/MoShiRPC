#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>

#include "common/timer.hpp"
#include "common/time_stamp.hpp"

using moshi::Timer;
using moshi::Alarm;
using std::chrono::milliseconds;
using std::chrono::steady_clock;
using std::chrono::time_point;

bool Alarm::IsExpired() const {
    auto now_timepoint = steady_clock::now();
    return now_timepoint >= next_timepoint_;
}

void Alarm::Reset() {
    next_timepoint_ = steady_clock::now() + delay_;
}

time_point<steady_clock> Alarm::GetNextTimePoint() const {
    return next_timepoint_;
}

void Timer::AddMsTask(uint delay,
                    std::function<void ()> callback,
                    bool is_loop) {
    milliseconds delay_ms(delay);
    auto next_timepoint = steady_clock::now() + delay_ms;
    
    {
        std::lock_guard<std::mutex> locker(mtx_);
        q_.push({next_timepoint, callback, delay_ms, is_loop});
    }
    cv_.notify_all();
}

void Timer::Start() {
    if(running_.exchange(true)) {
        return;
    }

    t_ = std::thread([&]() {
        while(running_) {
            time_point<steady_clock> next_timepoint;
            std::unique_lock<std::mutex> locker(mtx_);
            if(!q_.empty())
                next_timepoint = q_.top().GetNextTimePoint();
            else
                next_timepoint = steady_clock::now() + milliseconds(1); // 1ms后重新进行检查
            cv_.wait_until(locker, next_timepoint);
            HandleAlarm_();
        }
    });
}

void Timer::Stop() {
    running_ = false;
    t_.join();
}

void Timer::Clear() {
    while(!q_.empty()) {
        auto t = q_.top();
        q_.pop();
        t(); // 提前进行处理，可能其中有资源释放相关的内容
    }
}

void Timer::HandleAlarm_() {
    auto now = steady_clock::now();
    while(!q_.empty()) {
        if(q_.top().GetNextTimePoint() > now) {
            break;
        }
        
        auto temp = q_.top();
        q_.pop();

        temp();
        
        if(temp.IsLooped()) {
            temp.Reset();
            q_.push(temp);
        }
    }
}