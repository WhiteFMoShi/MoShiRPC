#include <memory>
#include <string>

#include "log.h"
#include "logConfig/logConfig.h"
#include "logFormat/logFormat.h"

Log& Log::getInstance() {
    static Log log_; // 静态局部变量
    return log_;
}

Log::Log() : log_config_(LogConfig::getConfig()) {
    pool_ = std::make_unique<ThreadPool>(log_config_.threadNumber(), log_config_.usingThreadpool());
}

void Log::addLog(LogLevel level, std::string module, const std::string& msg) {
}