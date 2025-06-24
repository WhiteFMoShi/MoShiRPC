#pragma once

// 用于动态库的API导出 但是不知道实际上的效果如何
// #define  __attribute__((visibility("default")))

#include <string>
#include <memory>

enum class LogLevel : int {
    Debug = 1,
    Info,
    Warning,
    Error,
    Critical // 严重错误
};

class Log {
public:
    static Log& getInstance();

    /**
    * @brief Add a new Entry to Log's queue.
    * @param level The entry Level, can checking in LogLevel.
    * @param module which module add this entry.
    * @param msg The message of this entry.
    */
    void addLog(LogLevel level, std::string module, const std::string& msg);
    
    /**
    * @brief Close the threadpool, cleaning all entry in queue.
    * @warning This function shouldn't called derictly, because the open function haven't implement.
    */
    void close();

private:
    // PIMPL 封装
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
private:
    Log();
};

#define Debug(module, msg) void addLog(LogLevel::Debug, module, msg);
