#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "entry/log_entry.hpp"

class LogFileManager {
public:
    // 构造日志路径
    LogFileManager();
    
    ~LogFileManager();

    void writeInFile(const LogEntry&);
private:
    std::string log_dir_; // 末尾带有"/"
    std::mutex manager_mtx;

    using LogFilePath = std::string;
    std::map<LogFilePath, std::tuple<std::shared_ptr<std::ofstream>, std::shared_ptr<std::mutex>>> manager_; // 其中的数据如何进行管理？何时进行删除？不能一直膨胀
};
