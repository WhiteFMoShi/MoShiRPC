#pragma once

#include <string>

class LogFileManager {
public:
    // 构造日志路径
    LogFileManager();

    // 创建当天的日志文件（根据要输出的日志的时间戳来）
    void createLogFile();
private:
    std::string log_dir_;
};
