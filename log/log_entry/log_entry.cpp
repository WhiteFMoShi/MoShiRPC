#include <string>

#include "log_entry.hpp"
#include "../utils/time_stamp.hpp"
#include "log_format/log_format.hpp"

// #define LOGENTRY_DEBUG

LogEntry::LogEntry(LogLevel level, const std::string& module, const std::string& msg) {
    date_ = TimeStamp::date();

    LogFormat fmt;
    msg_ = fmt.makeLogln(level, module, TimeStamp::now(), msg);
#ifdef LOGENTRY_DEBUG
    std::cout << "LogEntry's msg is: " << msg_;
    std::cout << "LogEntry's create time is: " << TimeStamp::now() << std::endl;
#endif
}

const std::string LogEntry::date() const {
    return date_;
}

const std::string LogEntry::getMsg() const {
    return msg_;
}
