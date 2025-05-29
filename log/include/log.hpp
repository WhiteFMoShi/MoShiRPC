#pragma once

#include <atomic>
#include <condition_variable>
#include <vector>
#include <string>
#include <mutex>
#include <future>
#include <condition_variable>
#include <vector>

#include "config/log_config.hpp"
#include "file/log_file_manager.hpp"
#include "utils/log_level.hpp"
#include "utils/log_queue.hpp"

class Log {
public:
    static Log& getInstance();

    // 向队列中添加日志条目
    void addLog(LogLevel level, std::string module, const std::string& msg);
    void close();

private:
    // 仿函数
    class LogWriter {
    public:
        std::future<bool> operator()(Log& log);
    };

    const LogConfig& log_config_;
    std::atomic<bool> flag_;

    LogQueue buffer_;
    std::vector<std::future<bool>> pool_;
    LogFileManager manager_;

    std::mutex mtx_; // 读锁
    std::condition_variable cv_;

    /*
        确保addLog在所有线程启动之后再添加
        这样不会出现数据丢失的问题
    */
    std::atomic<int> ready_count{0};
    std::condition_variable ready_cv;
    std::mutex ready_mtx;
private:
    Log();
};
