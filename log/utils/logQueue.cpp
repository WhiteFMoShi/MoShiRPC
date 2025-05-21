#include <mutex>

#include "logQueue.h"

void LogQueue::push(LogEntry mem) {
    std::lock_guard<std::mutex> locker(mtx_);
    q_.push(mem);
}

LogEntry LogQueue::front_and_pop() {
    std::lock_guard<std::mutex> locker(mtx_);
    LogEntry ret = q_.front();
    q_.pop();
    return ret;
}

bool LogQueue::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return q_.empty();
}