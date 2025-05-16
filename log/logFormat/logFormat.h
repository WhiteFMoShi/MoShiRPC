#pragma once

#include <sstream>
#include <string>

#include "timeStamp.h"

class LogFormat {
public:
    enum class Level : int {
        Debug = 1,
        Info,
        Warning,
        Error,
        Critical // 严重错误
    };

    const std::string make_log(Level level, std::string module, const std::string& msg);
private:
    TimeStamp st_;
};