#pragma once

#include <bits/types/struct_tm.h>
#include <cstdint>
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
#include <atomic>
#include <unordered_set>

#include "time_stamp.hpp"

namespace moshi {


struct Alarm {
public:
    Alarm(std::chrono::time_point<std::chrono::steady_clock> next_timepoint,
        std::function<void()> cb,
        std::chrono::milliseconds ms,
        uint64_t id
    ) : next_timepoint_(next_timepoint),
        cb_(cb), delay_(ms), id_(id) {}

    void operator()() { cb_(); }
    bool IsExpired() const;
    void Reset();
    uint64_t GetID() const { return id_; }

    std::chrono::time_point<std::chrono::steady_clock> 
                            GetNextTimePoint() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> next_timepoint_;
    std::function<void()> cb_;
    std::chrono::milliseconds delay_;
    uint64_t id_;
};


class Timer {
public:
    Timer() = default;
    ~Timer() { Stop(); }

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
    uint64_t FetchID_();

private:
    std::priority_queue<Alarm,
        std::vector<Alarm>,
        TimerCmp> q_; // 构建一个小顶堆
    std::unordered_set<uint64_t> loop_alarms_; // 某个定时器是否需要重入

    std::atomic<bool> running_{false};
    std::condition_variable cv_;
    std::mutex q_mtx_;
    std::mutex hash_mtx_;
    std::atomic<uint64_t> alarm_id_count_{1};

    std::thread t_;
};

} // namespace moshi
