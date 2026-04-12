#pragma once

#include <bits/types/struct_tm.h>
#include <queue>
#include <thread>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <type_traits>
#include <memory>

#include "time_stamp.hpp"

namespace moshi {

struct Alarm {
public:
    Alarm(std::chrono::time_point<std::chrono::steady_clock> next_timepoint,
        std::function<void()> cb,
        std::chrono::milliseconds ms,
        bool is_loop
    ) : next_timepoint_(next_timepoint),
        cb_(cb), delay_(ms), is_loop_(is_loop) {}
    void operator()() { cb_(); }
    bool IsExpired() const;
    bool IsLooped() const { return is_loop_; };
    void Reset();

    std::chrono::time_point<std::chrono::steady_clock> 
                            GetNextTimePoint() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> next_timepoint_;
    std::function<void()> cb_;
    std::chrono::milliseconds delay_;
    bool is_loop_;
};


class Timer {
public:
    Timer() = default;
    ~Timer() = default;

    void AddMsTask(uint delay,
            std::function<void()> callback,
            bool loop = false);
    void Start();
    void Stop();
    void Clear();

private:
    struct TimerCmp {
        bool operator()(const Alarm& lhs, const Alarm& rhs) {
            return lhs.GetNextTimePoint() > rhs.GetNextTimePoint();
        }
    };
    void HandleAlarm_();

private:
    std::priority_queue<Alarm,
        std::vector<Alarm>,
        TimerCmp> q_; // 构建一个小顶堆

    std::atomic<bool> running_{false};
    std::condition_variable cv_;
    std::mutex mtx_;

    std::thread t_;
};

} // namespace moshi