#pragma once

#include <string>

namespace moshi {

class BaseLog {
public:
    virtual ~BaseLog() = default;

    virtual void Debug(const std::string& module, const std::string& msg) = 0;
    virtual void Info(const std::string& module, const std::string& msg) = 0;
    virtual void Warn(const std::string& module, const std::string& msg) = 0;
    virtual void Error(const std::string& module, const std::string& msg) = 0;
    virtual void Critical(const std::string& module, const std::string& msg) = 0;
};

} // namespace moshi
