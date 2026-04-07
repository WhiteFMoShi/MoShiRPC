#include <chrono>
#include <sstream>

#include "time_stamp.hpp"

// #define TIMESTAMP_DEBUG

std::string TimeStamp::date() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << tm.tm_year + 1900 << "_" << tm.tm_mon + 1 << "_" << tm.tm_mday;
    return oss.str();
}

std::string TimeStamp::now() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << tm.tm_year + 1900 << "/" << tm.tm_mon + 1 << "/" << tm.tm_mday << " "
        << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec;
    return oss.str();
}