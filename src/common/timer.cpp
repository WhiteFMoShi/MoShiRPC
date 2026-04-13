#include <algorithm>
#include <chrono>
#include <cstdint>
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
    uint64_t id;
    
    {
        std::lock_guard<std::mutex> q_locker(q_mtx_);
        Alarm am = {next_timepoint, callback, delay_ms, FetchID_()};
        q_.push(am);
        id = am.GetID();
    }

    if(is_loop) {
        std::lock_guard<std::mutex> hash_locker(hash_mtx_);
        loop_alarms_.insert(id);
    }
    cv_.notify_one();
}

void Timer::Start() {
    if(running_.exchange(true)) {
        return;
    }

    t_ = std::thread([&]() {
        while(running_) {
            time_point<steady_clock> next_timepoint;
            std::unique_lock<std::mutex> locker(q_mtx_);
            if(!q_.empty())
                next_timepoint = q_.top().GetNextTimePoint();
            else
                next_timepoint = steady_clock::now() + milliseconds(1); // 1ms后重新进行检查
            cv_.wait_until(locker, next_timepoint);
            locker.unlock(); // 防止死锁
            HandleAlarm_();
        }
    });
}

void Timer::Stop() {
    if(t_.get_id() == std::this_thread::get_id())
        return; // 不允许自调用Stop()

    if(!running_.load())
        return;

    running_ = false;

    Clear();
    cv_.notify_all();
    if(t_.joinable())
        t_.join();
}

void Timer::Clear() {
    std::lock_guard<std::mutex> locker(q_mtx_);
    {
        std::lock_guard<std::mutex> hash_locker(hash_mtx_);
        loop_alarms_.clear();
    }
    while(!q_.empty())
        q_.pop();

    cv_.notify_one();
}

void Timer::HandleAlarm_() {
    std::unique_lock<std::mutex> q_locker(q_mtx_, std::defer_lock);

    while(running_) {
        q_locker.lock();
        if (q_.empty()) {
            q_locker.unlock();
            return;
        }

        auto am = q_.top();
        auto now = steady_clock::now();
        if(am.GetNextTimePoint() > now) {
            q_locker.unlock();
            break;
        }
        q_.pop();
        q_locker.unlock();

        am();
        uint64_t id = am.GetID();
        
        std::unique_lock<std::mutex> hash_locker(hash_mtx_);
        if(loop_alarms_.find(id) != loop_alarms_.end() && running_) {
            hash_locker.unlock();
            am.Reset();
            q_locker.lock();
            q_.push(am);
            q_locker.unlock();
        }
    }
}

uint64_t Timer::FetchID_() {
    return alarm_id_count_++;
}
