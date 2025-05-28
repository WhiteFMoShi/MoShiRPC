#pragma once

#include <any>
#include <string>
#include <unordered_map>
#include <vector>

// 只适合有一个config，所以单例模式是最好的
class LogConfig {
public:
    // 是否使用线程池
    bool usingThreadpool() const;
    // 设定的线程数
    int threadNumber() const;
    // 日志存储文件夹（前有'/'）
    std::string logDir() const;
    // 获取makefile所在的目录（默认直接make而不使用-file指定）
    std::string getWorkSpace() const;

    // 单例获取
    static LogConfig& getConfig();
private:
    /*
        Log.Config的信息，用于找到配置文件
    */
    std::string workspace_; // 文件路径
    std::string config_file_name_; // 文件名
    std::string config_file_full_path_; // config文件全路径
    std::string conf_dir_; // 配置文件所在目录

    // log具有的配置信息
    std::unordered_map<std::string, std::any> config_ {
        {"asynchronous", true},
        {"thread_number", 4},
        {"log_dir_relative_path", std::string("/Log")}, // 强制使用string进行存储
    };

    // 键顺序（确保文件中默认的、具有相关性的配置信息是相邻的）
    std::vector<std::string> sequence_ {
        "asynchronous",
        "thread_number",
        "log_dir_relative_path"
    };

private:
    LogConfig(std::string conf_dir = "/conf", std::string config_file_name = "log.config");
    LogConfig(LogConfig&&) = delete;

    LogConfig operator=(LogConfig&&) = delete;
    
    // 获取log.config中的配置信息
    bool setConfig_();
    // 去除字符串前后的空格
    const std::string trim_(std::string& str); 
    // 将字符串统一转换成小写
    void toLower_(std::string& str);
    
    std::any keyToValue_(const std::string&) const;
};

