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
private:
    void clean_up_(const std::string& log_file_name);
    std::string log_dir_;
    std::mutex manager_mtx;

    using LogFilePath = std::string;
    std::unordered_map<std::string, std::tuple<
        std::shared_ptr<std::ofstream>,
        std::shared_ptr<std::mutex>,
        std::unique_ptr<AdvancedConditionalTimer>
    >> manager_;

};
