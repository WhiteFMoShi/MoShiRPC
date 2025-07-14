#pragma once

#include <string>
#include <memory>

namespace MoShi {

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
private:
    Log();
    ~Log();

    /**
    * @brief Close the threadpool, cleaning all entry in queue.
    * @warning This function shouldn't called derictly, because the open function haven't implement.
    */
    void close();
};

} // namespace MoShi