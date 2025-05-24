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

#include "logConfig.h"

// #define LOGCONFIG_DEBUG

bool LogConfig::usingThreadpool() const {
    // 不能使用"[]",它是非const的，因为会默认插入被访问但是不存在的键
    // 此处需要使用at，因为at不存在这一点，它只是一个访问器，并且它具有越界检查
    // 我有一个at包装器(keyToValue)
    return std::any_cast<bool>(keyToValue_("asynchronous"));
}

int LogConfig::threadNumber() const {
    return std::any_cast<int>(keyToValue_("thread_number"));
}

std::string LogConfig::logDir() const {
    return std::any_cast<std::string>(keyToValue_("log_dir_relative_path"));
}

// 单例接口
LogConfig& LogConfig::getConfig() {
    static LogConfig config;
    return config;
}

LogConfig::LogConfig(std::string conf_dir, std::string configfile_name) {
    workspace_ = getWorkSpace();
    config_file_name_ = configfile_name;
    conf_dir_ = conf_dir;
    config_file_full_path_ = workspace_ + conf_dir + "/" + config_file_name_; // log.config路径

    setConfig_();
}

std::string LogConfig::getWorkSpace() const {
    std::string workdir(std::filesystem::current_path());
#ifdef  LOGCONFIG_DEBUG
    std::cout << "Workspace: " << workdir << std::endl;
#endif
    return workdir;
}

bool LogConfig::setConfig_() {
    std::fstream file_stream;

    file_stream.open(config_file_full_path_, std::ios_base::in); // 只读模式
#ifdef  LOGCONFIG_DEBUG
    std::cout << "Config file path: " << config_file_full_path_ << std::endl;
#endif
    bool flag = false; // 用于标识用户自定义的log.config是否被读取

    // 文件成功打开了
    if(file_stream.is_open()) {
        flag = true;
        char line[256];
        while(file_stream.getline(line, 256, '\n')) {
            std::stringstream ss(line);
            std::string key, value_str; // Renamed value to value_str to avoid confusion
            if(std::getline(ss, key, ':') && \
                std::getline(ss, value_str)) {
                key = trim_(key), value_str = trim_(value_str);
#ifdef  LOGCONFIG_DEBUG
                std::cout << "Parse: " << key << ": " << value_str << std::endl; // Debug info
                // It's safer to check type after potential modification or use a temporary any
                // For now, assuming config_.at(key) refers to the type defined in header for this check
                if (config_.count(key)) { // Ensure key exists before at()
                    std::cout << "value type_info (default): " << keyToValue_(key).type().name() << std::endl;
                }
                std::cout << "---------------" << std::endl;
#endif
            }
            toLower_(key); // 键全部变为小写
            
            if (!config_.count(key)) { // Skip if key is not in predefined config
                continue;
            }

            if(keyToValue_(key).type() == typeid(int))
                config_[key] = std::stoi(value_str);
            else if(keyToValue_(key).type() == typeid(std::string)) { // 修改: 检查 std::string 类型
                config_[key] = value_str; // 修改: 直接存储 std::string
#ifdef  LOGCONFIG_DEBUG
                std::cout << "in string setting that > " << key << " : "<< value_str << std::endl;
#endif
            }
            else if(keyToValue_(key).type() == typeid(bool))
                config_[key] = (value_str == "true" ? true : false);
        }

        file_stream.close();
    }
    else {
        file_stream.close(); // 关闭文件后重新打开

        std::filesystem::path p(config_file_full_path_);
        std::filesystem::path parent_p(p.parent_path()); // 用于目录检查
        if(!std::filesystem::exists(parent_p) && !std::filesystem::create_directory(parent_p)) { // 目录不存在就创建目录
            throw std::runtime_error("Create conf path error!!!"); // 目录创建失败（抛出异常）
        }

        file_stream.open(config_file_full_path_, std::ios_base::out);
        // 向其中添加配置信息
        for(const auto& key : sequence_) {
            file_stream << key << ": ";
            auto value = keyToValue_(key);
            if(value.type() == typeid(bool)) {
                file_stream << (std::any_cast<bool>(value) ? "true" : "false") << std::endl;
            }
            else if(value.type() == typeid(std::string)) // 修改: 检查 std::string 类型
                file_stream << std::any_cast<std::string>(value) << std::endl; // 修改: 转换并输出 std::string
            else if(value.type() == typeid(int))
                file_stream << std::any_cast<int>(value) << std::endl;
        }

        file_stream.close();
    }
    if(flag) {
        std::cout << "Config file read succ!!!" << std::endl;
    }
    else {
        std::cerr << "Config file doesn't exist or path error, auto touch new one!!!" << std::endl;
    }

    std::cout << "config setting is:" << std::endl;
    for(const auto& key : sequence_) {
        std::cout << "\t" << key << " : ";
        auto value = config_.at(key);
        if(value.type() == typeid(bool)) {
            std::cout << (std::any_cast<bool>(value) ? "true" : "false") << std::endl;
        }
        else if(value.type() == typeid(std::string))
            std::cout << std::any_cast<std::string>(value) << std::endl;
        else if(value.type() == typeid(int))
            std::cout << std::any_cast<int>(value) << std::endl;
        else {
            std::runtime_error("value type can't match");
        }
    }

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

std::any LogConfig::keyToValue_(const std::string& key) const {
    return config_.at(key);
}