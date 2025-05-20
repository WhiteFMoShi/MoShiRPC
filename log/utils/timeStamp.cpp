#include "timeStamp.h"

// #define TIMESTAMP_DEBUG

TimeStamp::TimeStamp() {
    const auto time_p = std::chrono::system_clock::now();
    const time_t time(std::chrono::system_clock::to_time_t(time_p));
    t_ = std::localtime(&time);
}

const std::string TimeStamp::date() const {
    std::ostringstream oss;
    oss << t_->tm_year + 1900 << "_" << t_->tm_mon + 1 << "_" \
    << t_->tm_mday;
#ifdef TIMESTAMP_DEBUG
    std::cout << "The date is: " << oss.str() << std::endl;
#endif
    return oss.str();
}
// 获取当前的时间戳，并进行格式化
const std::string TimeStamp::now() const {

    std::ostringstream oss;
    oss << t_->tm_year + 1900 << "/" << t_->tm_mon + 1 << "/" \
    << t_->tm_mday << " "  << t_->tm_hour << ":" << t_->tm_min << ":" \
    << t_->tm_sec;

#ifdef TIMESTAMP_DEBUG
    std::cout << "Now is: " << oss.str() << std::endl;
#endif

    return oss.str();
}