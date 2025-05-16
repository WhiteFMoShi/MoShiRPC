#pragma once

#include <any>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

class LogConfig {
public:
    LogConfig(std::string conf_dir = "conf", std::string configfile_name = "log.config");
    LogConfig operator=(LogConfig&&) = delete;
    LogConfig(const LogConfig&) = delete;
    LogConfig(LogConfig&&) = delete;

    bool using_threadpool();
private:
    /*
        Log.Config的信息，用于找到配置文件
    */
    std::string path_; // 文件路径
    std::string name_; // 文件名
    std::string fullPath_; // config文件全路径
    std::string conf_dir_; // 配置文件所在目录

    /*
        log文件相关信息
    */
    std::string logDir;

    // log具有的配置信息
    std::unordered_map<std::string, std::any> config_ {
        {"using_threadpool", false},
        {"log_dir_relative_path", "/Log"},
    };

    // 键顺序（确保文件中默认的、具有相关性的配置信息是相邻的）
    std::vector<std::string> sequence_ {
        "using_threadpool",
        "log_dir_relative_path"
    };

private:
    std::string getWorkSpace_(); // 获取makefile所在的目录（默认直接make而不使用-file指定）
    bool getConfig_(); // 获取log.config中的配置信息

    const std::string trim_(std::string& str); // 去除字符串前后的空格
    void toLower_(std::string& str); // 将字符串统一转换成小写 
};

