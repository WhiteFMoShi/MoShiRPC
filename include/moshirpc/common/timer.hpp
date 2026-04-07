#pragma once

#include <queue>
#include <thread>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <chrono>
#include <atomic>
#include <type_traits>

namespace moshi {

struct Clock {
};

class Timer {
public:
    Timer() = default;
    ~Timer() = default;

    /**
     * @brief 设定一个时常为timeout的计时器
     * 
     * @tparam T 
     * @param timeout 
     * @param callback 
     * @return true 
     * @return false 
     */
    template<class T>
    bool add_task(T timeout, std::function<void()> callback);

    void start();
    void stop();

private:
    std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer>> q_; // 构建一个小顶堆

    std::atomic<bool> flag_{false};
};

} // namespace moshi