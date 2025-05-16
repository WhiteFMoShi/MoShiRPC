#include <any>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
// #include <unistd.h>

#include "logConfig.h"

// #define LOGCONFIG_DEBUG

LogConfig::LogConfig(std::string conf_dir, std::string configfile_name) {
    path_ = getWorkSpace_();
    name_ = configfile_name;
    conf_dir_ = conf_dir;
    fullPath_ = path_ + "/" + conf_dir + "/" + name_; // log.config路径

    getConfig_();
}

bool LogConfig::using_threadpool() {
    return std::any_cast<bool>(config_["using_threadpool"]);
}

// 使用标准库的、更加通用的实现
std::string LogConfig::getWorkSpace_() {
    std::string workdir(std::filesystem::current_path().c_str());
#ifdef  LOGCONFIG_DEBUG
    std::cout << "workdir: " << workdir << std::endl;
#endif
    return workdir;
}

bool LogConfig::getConfig_() {
    std::fstream file_stream;

    file_stream.open(fullPath_, std::ios_base::in); // 只读模式
#ifdef  LOGCONFIG_DEBUG
            std::cout << "Config file path: " << fullPath_ << std::endl;
#endif
    bool flag = false; // 用于标识用户自定义的log.config是否被读取

    // 文件成功打开了
    if(file_stream.is_open()) {
        flag = true;
        char line[256];
        while(file_stream.getline(line, 256, '\n')) {
#ifdef  LOGCONFIG_DEBUG
            std::cout << "line: " << line << std::endl;
#endif
            std::stringstream ss(line);
            std::string key, value;
            if(std::getline(ss, key, ':') && \
                std::getline(ss, value)) {
                key = trim_(key), value = trim_(value);
#ifdef  LOGCONFIG_DEBUG
                std::cout << "Parse: " << key << ": " << value << std::endl; // Debug info
#endif
            }
            toLower_(key), toLower_(value);
            config_[key] = (value == "true");
        }

        file_stream.close();
    }
    else {
        file_stream.close(); // 关闭文件后重新打开

        std::filesystem::path p(fullPath_);
        std::filesystem::path parent_p(p.parent_path()); // 用于目录检查
        if(!std::filesystem::exists(parent_p) && !std::filesystem::create_directory(parent_p)) { // 目录不存在就创建目录
            throw std::runtime_error("Create conf path error!!!"); // 目录创建失败（抛出异常）
        }

        file_stream.open(fullPath_, std::ios_base::out);
        // 向其中添加配置信息
        for(const auto& key : sequence_) {
            file_stream << key << ": ";
            auto value = config_[key];
            if(value.type() == typeid(bool)) {
                file_stream << (std::any_cast<bool>(value) ? "true" : "false") << std::endl;
            }
            else if(value.type() == typeid(const char*))
                file_stream << std::any_cast<const char*>(value) << std::endl;
        }

        file_stream.close();
    }
#ifdef LOGCONFIG_DEBUG
    if(flag) {
        std::cout << "Config file read succ!!!" << std::endl;
    }
    else {
        std::cerr << "Config file doesn't exist or path error, auto touch new one!!!" << std::endl;
    }
#endif

    return flag;
}

const std::string LogConfig::trim_(std::string& str) {
    size_t n = str.size();
    size_t begin = 0, end = n;
    while(begin < n && str[begin] == ' ')
        begin++;
    while(end >= 0 && end >= begin && str[end] == ' ')
        end--;
    if(begin < end)
        return str.substr(begin, end - begin);
    return "";
}

void LogConfig::toLower_(std::string& str) {
    for(auto& c : str)
        c = std::tolower(c);
}
