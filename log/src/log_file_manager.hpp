#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "log_entry.hpp"
#include "timer.hpp"

class LogFileManager {
public:
    // 构造日志路径
    LogFileManager();
    
    ~LogFileManager();

    void writeInFile(const LogEntry&);
    void writeInFile(LogEntry&& entry);
private:
    std::string log_dir_; // 末尾带有"/"
    std::mutex manager_mtx;

    using LogFilePath = std::string;
    std::unordered_map<std::string, std::tuple<
        std::shared_ptr<std::ofstream>,      // 第1个：文件流
        std::shared_ptr<std::mutex>,         // 第2个：互斥锁
        std::unique_ptr<AdvancedConditionalTimer>  // 第3个：定时器
    >> manager_;
};