#pragma once

// 用于动态库的API导出
#define LOG_API __attribute__((visibility("default")))

#include <string>
#include <memory>

enum class LogLevel : int {
    Debug = 1,
    Info,
    Warning,
    Error,
    Critical // 严重错误
};

class LOG_API Log {
public:
    LOG_API static Log& getInstance();

    // 向队列中添加日志条目
    LOG_API void addLog(LogLevel level, std::string module, const std::string& msg);
    LOG_API void close();

private:
    // PIMPL 封装
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
private:
    Log();
};
