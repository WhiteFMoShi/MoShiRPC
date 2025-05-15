#pragma once

#include <any>
#include <string>
#include <fstream>
#include <unistd.h>
#include <unordered_map>

class LogConfig {
public:
    // 获取makefile所在的目录（默认直接make而不使用-file指定）
    std::string getWorkSpace();

    LogConfig(std::string name = "log.config") {
        path_ = getWorkSpace();
        name_ = name;

        fullPath_ = path_ + "/" + name_;
    }
    bool getConfig();
private:
    /*
        Log.Config的信息，用于找到配置文件
    */
    std::string path_; // 文件路径
    std::string name_; // 文件名
    std::string fullPath_; // config文件全路径

    /*
        log文件相关信息
    */
    std::string logDir;
    std::string logName; // Log文件的名字

    // log具有的配置信息
    std::unordered_map<std::string, std::any> config_ {
        {"using_threadpool", false},
        {"log_dir", "/Log"},
    };

private:
    const std::string trim_(std::string& str); // 去除字符串前后的空格
    void toLower_(std::string& str); // 将字符串统一转换成小写 
};

