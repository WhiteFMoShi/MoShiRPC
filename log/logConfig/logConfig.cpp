#include <any>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include "logConfig.h"

// #define LOGCONFIG_DEBUG

std::string LogConfig::getWorkSpace() {
    char path[4096]; // Linux中，完整路径的最大长度
    try {
        if(nullptr == getcwd(path, sizeof(path)))
            throw std::runtime_error("Get WorkSpace Error!!!");
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return "";
    }
    return std::string(path);
}

bool LogConfig::getConfig() {
    std::fstream fs;
    fs.open(fullPath_, std::ios_base::in); // 只读模式
    bool flag = false; // 用于标识用户自定义的log.config是否被读取

    // 文件成功打开了
    if(fs.is_open()) {
        flag = true;
        char line[256];
        while(fs.getline(line, 256, '\n')) {
#ifdef  LOGCONFIG_DEBUG
            std::cout << "line: " << line << std:: endl;
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

        fs.close();
    }
    else {
        fs.close(); // 关闭文件后重新打开
        fs.open(fullPath_, std::ios_base::out);

        // 向其中添加配置信息
        for(auto& [key, value] : config_) {
            fs << key << ": ";
            if(value.type() == typeid(bool)) {
                fs << (std::any_cast<bool>(value) ? "true" : "false") << std::endl;
            }
            else if(value.type() == typeid(const char*))
                fs << std::any_cast<const char*>(value) << std::endl;
        }

        fs.close();
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
