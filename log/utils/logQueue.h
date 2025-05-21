#pragma once

#include <mutex>
#include <queue>

#include "../logEntry/logEntry.h"

// 线程安全队列
class LogQueue {
public:
    bool empty();
    void push(LogEntry);
    LogEntry front_and_pop();
private:
    std::queue<LogEntry> q_;
    std::mutex mtx_;
};