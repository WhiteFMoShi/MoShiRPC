// #include <mutex>

// #include "logQueue.h"

// void LogQueue::push(LogEntry mem) {
//     std::lock_guard<std::mutex> locker(mtx_);
//     q_.push(mem);
// }

// LogEntry LogQueue::front_and_pop() {
//     std::lock_guard<std::mutex> locker(mtx_);
//     LogEntry ret = q_.front();
//     q_.pop();
//     return ret;
// }

// bool LogQueue::empty() {
//     std::lock_guard<std::mutex> locker(mtx_);
//     return q_.empty();
// }

// int LogQueue::size() {
//     std::lock_guard<std::mutex> locker(mtx_);
//     return q_.size();
// }

#include "logQueue.h"
#include <stdexcept>

// 检查队列是否为空
bool LogQueue::empty() {
    std::lock_guard<std::mutex> lock(push_mtx_);
    std::lock_guard<std::mutex> lock2(pop_mtx_);
    return q_.empty();
}

// 向队列中推入一个元素
void LogQueue::push(LogEntry entry) {
    std::lock_guard<std::mutex> lock(push_mtx_);
    q_.push(entry);
}

// 获取队列的大小
int LogQueue::size() {
    std::lock_guard<std::mutex> lock(push_mtx_);
    std::lock_guard<std::mutex> lock2(pop_mtx_);
    return q_.size();
}

// 获取队列头部元素并移除
LogEntry LogQueue::front_and_pop() {
    std::lock_guard<std::mutex> lock(pop_mtx_);
    if (q_.empty()) {
        // 这里可以根据实际需求处理队列为空的情况，例如抛出异常或返回默认构造的 LogEntry
        throw std::runtime_error("Queue is empty!!!");
    }
    LogEntry front = q_.front();
    q_.pop();
    return front;
}