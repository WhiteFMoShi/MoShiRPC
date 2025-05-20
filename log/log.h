#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>
#include <vector>

#include "logEntry/logEntry.h"
#include "logConfig/logConfig.h"
#include "../threadpool/threadPool.h"
#include "logFormat/logFormat.h"
#include "utils/logLevel.h"

class Log {
public:
    static Log& getInstance();

    void addLog(LogLevel level, std::string module, const std::string& msg); // 添加要写的日志
    // void addTask(std::string&&);
private:
    // 实际处理buffer_
    class LogWriter {
    public:
        std::future<int> operator()(std::vector<std::string>& buffer_);
    };
    
    std::atomic<bool> flag_;
    const LogConfig& log_config_;

    std::vector<LogEntry> buffer_; // 缓冲队列
    std::unique_ptr<ThreadPool> pool_; // 智能指针RAII

    std::mutex mtx_;
    std::condition_variable cv_;

private:
    Log();
};
