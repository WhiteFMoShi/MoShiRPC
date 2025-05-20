#include <ctime>
#include <filesystem>
#include <iostream>

#include "logFileManager.h"
#include "../logConfig/logConfig.h"

#define LOGFILEMANAGER_DEBUG

LogFileManager::LogFileManager() {
    const LogConfig& config = LogConfig::getConfig();

    log_dir_ = config.getWorkSpace() + "/" + config.logDir() + "/";

    std::cout << "Log Dir is" << log_dir_ << std::endl;
    if(std::filesystem::exists(log_dir_) == false && std::filesystem::create_directory(log_dir_))
        std::cout << "Log Dir create succ!!!" << std::endl;
    else
        std::cout << "Log Dir already exist!!!" << std::endl;
}

// void LogFileManager::createLogFile() {
//     // 先获取当前时间
//     const auto time_p = std::chrono::system_clock().now();
//     const time_t time = std::chrono::system_clock::to_time_t(time_p);
//     tm* t = std::localtime(&time);


// }