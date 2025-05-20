#pragma once

#include <ctime>
#include <iostream>
#include <iterator>
#include <string>
#include <chrono>
#include <sstream>

// #define TIMESTAMP_DEBUG

class TimeStamp {
public:
    // 获取当前的时间戳，并进行格式化
    const std::string now() {
        const auto time_p = std::chrono::system_clock::now();
        const time_t time(std::chrono::system_clock::to_time_t(time_p));
        tm* t = std::localtime(&time);

#ifdef TIMESTAMP_DEBUG
        std::cout << t->tm_year + 1900 << "/" << t->tm_mon + 1 << "/" \
            << t->tm_mday << " "  << t->tm_hour << ":" << t->tm_min << ":" \
            << t->tm_sec << std::endl;
#endif

        std::ostringstream oss;
        oss << t->tm_year + 1900 << "/" << t->tm_mon + 1 << "/" \
        << t->tm_mday << " "  << t->tm_hour << ":" << t->tm_min << ":" \
        << t->tm_sec;
        return oss.str();
    }
};