#pragma once

#include <mutex>
#include <string>

#include "../logEntry/logEntry.h"

class LogFileManager {
public:
    // 构造日志路径
    LogFileManager();

    // 创建当天的日志文件（根据要输出的日志的时间戳来）
    void createLogFile();

    void write(const LogEntry&);
private:
    std::string log_dir_;

    std::mutex mtx_; // 写入锁
};
