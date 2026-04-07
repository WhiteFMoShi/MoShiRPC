#include "mos_log.hpp"

namespace moshi {

void MosLog::Debug(const std::string& module, const std::string& msg) {
    log_obj_->add_log(LogLevel::Debug, module, msg);
}

void MosLog::Info(const std::string& module, const std::string& msg) {
    log_obj_->add_log(LogLevel::Info, module, msg);
}

void MosLog::Warn(const std::string& module, const std::string& msg) {
    log_obj_->add_log(LogLevel::Warning, module, msg);
}

void MosLog::Error(const std::string& module, const std::string& msg) {
    log_obj_->add_log(LogLevel::Error, module, msg);
}

void MosLog::Critical(const std::string& module, const std::string& msg) {
    log_obj_->add_log(LogLevel::Critical, module, msg);
}

} // namespace moshi
