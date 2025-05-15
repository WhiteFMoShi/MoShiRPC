#pragma once

#include <condition_variable>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <condition_variable>

#include "logConfig/logConfig.h"

class Log {
public:
    static Log& getInstance() {
        
    }
private:
    enum Level {
        Debug = 1,
        Info,
        Warning,
        Error,
        Critical // 严重错误
    };

    enum Mode {
        Sync,
        Async,
    };

    LogConfig log_config_;
    Level Level_; // 日志级别（不应该在这，应该是实际打印的时候使用）

    std::vector<std::string> buffer_; // 任务队列
    std::string name;

    std::mutex mtx_;
    std::condition_variable cv_;

    static Log* log_;

private:
    Log();
};