#pragma once

#include <string>

#include "/home/xfy/module/log/logEntry/logFormat/logFormat.h"
#include "../utils/logLevel.h"

class LogEntry {
public:
    LogEntry() = default;
    LogEntry(LogLevel level, const std::string& module, const std::string& msg);
    const std::string date() const;
    const std::string getMsg() const;
private:
    std::string date_; // 时间信息, yyyy_mm_dd
    std::string msg_; // 格式化的，要写入的数据, 带ln
};
