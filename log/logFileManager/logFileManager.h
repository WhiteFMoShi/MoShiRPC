#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../logEntry/logEntry.h"

class LogFileManager {
public:
    // 构造日志路径
    LogFileManager();



    void write(std::ofstream&, const LogEntry&);
private:
    std::string log_dir_;
    std::map<std::string, std::tuple<std::shared_ptr<std::ofstream>, std::shared_ptr<std::mutex>>> manager_; // 其中的数据如何进行管理？何时进行删除？不能一直膨胀

    // 创建当天的日志文件（根据要输出的日志的时间戳来）
    void create_log_file_();
};
