#pragma once

#include <sstream>
#include <string>

#include "log.hpp"

class LogFormat {
public:
    // 生成格式化的log字符串
    std::string makeLog(moshi::LogLevel level, const std::string& module, const std::string& time, const std::string& msg);

    // 尾部带有换行符的
    std::string makeLogln(moshi::LogLevel level, const std::string& module, const std::string& time, const std::string& msg);
private:
    // 进行流拼接
    void splicing_(std::ostringstream&, moshi::LogLevel level, const std::string& module, const std::string& time, const std::string& msg);
};
