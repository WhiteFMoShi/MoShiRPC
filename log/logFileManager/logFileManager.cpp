#include <ctime>
#include <filesystem>
#include <ios>
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "logFileManager.h"
#include "../logConfig/logConfig.h"

// #define LOGFILEMANAGER_DEBUG

LogFileManager::LogFileManager() {
    const LogConfig& config = LogConfig::getConfig();

    log_dir_ = config.getWorkSpace() + config.logDir() + "/";

    std::cout << "Log Dir is" << log_dir_ << std::endl;
    if(std::filesystem::exists(log_dir_) == false && std::filesystem::create_directories(log_dir_))
        std::cout << "Log Dir create success!!!" << std::endl;
    else
        std::cout << "Log Dir already exist!!!" << std::endl;
}

void LogFileManager::create_log_file_() {
    // 先获取当前时间
    const auto time_p = std::chrono::system_clock().now();
    const time_t time = std::chrono::system_clock::to_time_t(time_p);
    tm* tm = std::localtime(&time);

    std::stringstream ss;
    ss << tm->tm_year + 1900 << "_" << (tm->tm_mon + 1) << "_" << tm->tm_mday << ".log";
    std::string filename(ss.str());

#ifdef LOGFILEMANAGER_DEBUG
    std::cout << "Today's Log file name is: "<< filename << std::endl;
    std::cout << "The full path is: " << log_dir_  + filename << std::endl;
#endif
    std::fstream fs;
    fs.open(log_dir_ + filename, std::ios::app);
    if(!fs.is_open())
        throw std::runtime_error("Log File can't find and create!!!");
    fs.close();
}

void LogFileManager::write(std::ofstream& fs, const LogEntry& entry) {
    auto it = manager_.find(entry.date());
    if(it != manager_.end()) {
        auto& [ofs_ptr, mtx_ptr] = it->second;

        std::lock_guard<std::mutex> locker(*mtx_ptr);
        *ofs_ptr << entry.getMsg();
    }
    // 创建一个独特的ofs
    else {
        std::shared_ptr<std::ofstream> ofs_ptr;
        std::shared_ptr<std::mutex> mtx_ptr;

        manager_[entry.date()] = std::make_tuple(ofs_ptr, mtx_ptr);
        
    }
}