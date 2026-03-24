#pragma once

#include <string>

#include "base_log.hpp"
#include "log/include/log.hpp"

namespace moshi {

// Adapter: BaseLog(Target) -> moshi::Log(Adaptee)
class MosLog final : public BaseLog {
public:
    explicit MosLog(Log& log = Log::get_instance()) : log_obj_(&log) {}

    void Debug(const std::string& module, const std::string& msg) override;
    void Info(const std::string& module, const std::string& msg) override;
    void Warn(const std::string& module, const std::string& msg) override;
    void Error(const std::string& module, const std::string& msg) override;
    void Critical(const std::string& module, const std::string& msg) override;

private:
    Log* log_obj_;
};

} // namespace moshi
