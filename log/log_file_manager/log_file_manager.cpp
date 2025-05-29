#include <ctime>
#include <filesystem>
#include <ios>
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>

#include "log_file_manager.hpp"
#include "../log_config/log_config.hpp"

// #define LOGFILEMANAGER_DEBUG

LogFileManager::LogFileManager() {
    const LogConfig& config = LogConfig::getConfig();

    log_dir_ = config.getWorkSpace() + config.logDir() + "/";

    std::cout << "Log Dir is: " << log_dir_ << std::endl;
    if(std::filesystem::exists(log_dir_) == false && std::filesystem::create_directories(log_dir_))
        std::cout << "Log Dir create success!!!" << std::endl;
    else
        std::cout << "Log Dir already exist!!!" << std::endl;
}

LogFileManager::~LogFileManager() {
    for(auto& [key, ofs_and_mtx]: manager_) {
        auto& [ofs, mtx] = ofs_and_mtx;
        ofs->close();
    }
}


/*
    直接写入就可以，实际上是不需要创建的
*/
// void LogFileManager::create_log_file_(const LogEntry& entry) {

//     std::string filename(entry.date() + ".log");

// #ifdef LOGFILEMANAGER_DEBUG
//     std::cout << "Today's Log file name is: "<< filename << std::endl;
//     std::cout << "The full path is: " << log_dir_  + filename << std::endl;
// #endif
//     std::fstream fs;
//     fs.open(log_dir_ + filename, std::ios::app);
//     if(!fs.is_open())
//         throw std::runtime_error("Log File can't find and create!!!");
//     fs.close();
// }

void LogFileManager::writeInFile(const LogEntry& entry) {
#ifdef LOGFILEMANAGER_DEBUG
    std::cout << "Walk in LogFileManager::writeInFile" << std::endl;
#endif
    std::string log_file = log_dir_ + entry.date() + ".log";

    auto it = manager_.find(log_file);
    if(it == manager_.end()) {
        std::lock_guard<std::mutex> locker(manager_mtx);

        it = manager_.find(log_file); // 重新获取
        // 双重检查
        if(it == manager_.end()) {
            std::shared_ptr<std::ofstream> ofs_ptr = std::make_shared<std::ofstream>();
            std::shared_ptr<std::mutex> mtx_ptr = std::make_shared<std::mutex>();
            manager_[log_file] = std::make_tuple(ofs_ptr, mtx_ptr);

            std::cout << "Log File: " << log_file << " create succ!!!" << std::endl;
        }
    }

    it = manager_.find(log_file); // 重新获取
    if(it != manager_.end()) {
        auto& [ofs_ptr, mtx_ptr] = it->second;

        std::lock_guard<std::mutex> locker(*mtx_ptr);
        if(!ofs_ptr->is_open()) {
            ofs_ptr->open(log_file, std::ios::app);
        }
        *ofs_ptr << entry.getMsg();
    }
    else {
        throw std::runtime_error("log file create failure, please look logFileManager::writeInFile!!!");
    }
}