#pragma once

#include <memory>
#include <string>

namespace moshi {

class LogInterface {
public:
    virtual ~LogInterface() = default;

    virtual void Debug(const std::string& module, const std::string& msg) = 0;
    virtual void Info(const std::string& module, const std::string& msg) = 0;
    virtual void Warn(const std::string& module, const std::string& msg) = 0;
    virtual void Error(const std::string& module, const std::string& msg) = 0;
    virtual void Critical(const std::string& module, const std::string& msg) = 0;

    /**
     * @brief 获取单例日志对象
     * 
     * @return LogInterface& 
     */
    virtual LogInterface& get_instance() = 0;
private:
    std::unique_ptr<LogInterface> log_;
};    

} // namespace moshi
