#include "log.h"
#include "logConfig/logConfig.h"
#include "logFormat/logFormat.h"
#include <memory>
#include <mutex>
#include <string>

Log& Log::getInstance() {
    static Log log_; // 静态局部变量
    return log_;
}

Log::Log() : log_config_(LogConfig::getConfig()) {
    pool_ = std::make_unique<ThreadPool>(log_config_.threadNumber(), log_config_.usingThreadpool());
}

void Log::addLog(LogFormat::Level level, std::string module, const std::string& msg) {
    LogFormat fmt;
    const std::string str = fmt.makeLog(level, module, msg);
    {
        std::lock_guard<std::mutex> locker(mtx_);
        buffer_.emplace_back(str);
    }
}

void Log::writer_() {
    
}

void Log::thread_entrance_() {
    while(flag_) {
        std::unique_lock<std::mutex> locker(mtx_);
        if(buffer_.empty()) {
            cv_.wait(locker);
        }
        writer_();
    }
}