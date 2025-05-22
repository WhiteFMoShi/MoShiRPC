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
    TimeStamp();
    // 获取当前的日期: yyyy_mm_dd
    static std::string date();
    // 获取当前实时时间，精确到秒
    static std::string now();
};