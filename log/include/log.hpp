#pragma once

#ifndef MOSHI_LOG_H
#define MOSHI_LOG_H

#include <ctime>
#include <string>
#include <memory>

namespace moshi {

enum class LogLevel : int {
    Debug,
    Info,
    Warning,
    Error,
    Critical // 严重错误
};

class Log {
public:
    static Log& get_instance();

    /**
     * @brief 向日志队列中添加一条日志条目
     *
     * @param level 日志级别，可参考 LogLevel 枚举定义。
     * @param module 发起该日志的模块名称。
     * @param msg 日志的具体消息内容
     * @exception 如果在调用 close() 方法关闭日志后，再调用 addLog()，则会抛出 std::runtime_error 异常。
     *            该情况可能发生在多线程环境下。
     * @note 该函数可能会同时在terminal中打印日志，这取决于log.config是如何定义log行为的
     */
    void add_log(LogLevel level, const std::string& module, const std::string& msg);
    void add_log(LogLevel level, std::string&& module, std::string&& msg);

    /**
     * @brief 仅将日志信息打印到终端，而不写入到文件中
     * 
     * @param level 该条目的日志级别
     * @param module 模块信息
     * @param msg 日志信息
     */
    static void terminal_log(LogLevel level, const std::string& module, const std::string& msg);

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
     * @brief 关闭线程池，并清理队列中的所有日志条目。
     * 
     * @warning 此函数不应被直接调用，因为 open() 函数尚未实现。
     */
    void close();
};

} // namespace moshi


# endif