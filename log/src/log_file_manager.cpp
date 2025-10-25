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
#include "log_config.hpp"
#include "timer.hpp"

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
    for(auto& [key, ofs_and_mtx] : manager_) {
        auto& [ofs, mtx, timer] = ofs_and_mtx;
        ofs->close();
    }
}

void LogFileManager::writeInFile(const LogEntry& entry) {
#ifdef LOGFILEMANAGER_DEBUG
    std::cout << "Walk in LogFileManager::writeInFile" << std::endl;
#endif

    std::string log_file = log_dir_ + entry.date() + ".log";

    {
        std::lock_guard<std::mutex> locker(manager_mtx);

        auto it = manager_.find(log_file);
        if (it == manager_.end()) {
            // 文件不存在，创建资源
            auto ofs_ptr = std::make_shared<std::ofstream>();
            auto mtx_ptr = std::make_shared<std::mutex>();
            auto timer_ptr = std::make_unique<AdvancedConditionalTimer>();

            // 打开文件（追加模式）
            ofs_ptr->open(log_file, std::ios::app);
            if (!ofs_ptr->is_open()) {
                throw std::runtime_error("Failed to open log file: " + log_file);
            }

            // 启动一个 1分钟的定时器（原定是30分钟，但是这对于一个系统来说有点太久了）
            timer_ptr->start_min(1, [this, log_file]() {

                // 定时器超时回调（注意：运行在 worker 线程中！）
                std::lock_guard<std::mutex> cleanup_lock(this->manager_mtx);

                auto it_cleanup = this->manager_.find(log_file);
                if (it_cleanup != this->manager_.end()) {
#ifdef LOGFILEMANAGER_DEBUG
                    std::cout << "[AUTO-CLEAN] Log file '" << log_file 
                              << "' has no activity for 30 mins. Removing..." << std::endl;
#endif
                    // 关闭文件流（析构时自动关闭）
                    // 从 map 中移除该项
                    this->manager_.erase(it_cleanup);
                }
            });

            // 存入 manager_
            manager_[log_file] = std::make_tuple(ofs_ptr, mtx_ptr, std::move(timer_ptr));

#ifdef LOGFILEMANAGER_DEBUG
            std::cout << "Log File: " << log_file << " created & timer started!!!" << std::endl;
#endif
        }

        // 获取资源
        auto& [ofs_ptr, mtx_ptr, timer_ptr] = manager_[log_file];

        // 重置定时器，表示该文件还在使用中
        timer_ptr->reset_timer();

        // 写入日志内容
        {
            std::lock_guard<std::mutex> file_lock(*mtx_ptr);
            if (!ofs_ptr->is_open()) {
                ofs_ptr->open(log_file, std::ios::app);
            }
            if (ofs_ptr->is_open()) {
                *ofs_ptr << entry.getMsg();
            } else {
                throw std::runtime_error("Log file is not open: " + log_file);
            }
        }
    }
}