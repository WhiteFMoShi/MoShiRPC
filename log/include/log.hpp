#pragma once

#ifndef MOSHI_LOG_H
#define MOSHI_LOG_H

#include <string>
#include <memory>

namespace moshi {

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
    * @exception When calling addLog after using close(), it will throw out a std::runtimeerror.
                 It may happen Sin multiple-threads enviorment.
    */
    void addLog(LogLevel level, std::string module, const std::string& msg);

private:
    // PIMPL 封装
    struct Impl;
    std::unique_ptr<Impl> pimpl_;

    // 颜色常量
    static const std::string COLOR_DEBUG;
    static const std::string COLOR_INFO;
    static const std::string COLOR_WARNING;
    static const std::string COLOR_ERROR;
    static const std::string COLOR_CRITICAL;
    static const std::string COLOR_RESET;
private:
    Log();
    ~Log();

    /**
    * @brief Close the threadpool, cleaning all entry in queue.
    * @warning This function shouldn't called derictly, because the open function haven't implement.
    */
    void close();
};

} // namespace moshi

# endif