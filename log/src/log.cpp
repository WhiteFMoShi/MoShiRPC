#include <iostream>
#include <mutex>
#include <future>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "time_stamp.hpp"
#include "log.hpp"
#include "log_config.hpp"
#include "log_file_manager.hpp"
#include "log_queue.hpp"

using namespace moshi;

// #define LOG_DEBUG

const std::string Log::COLOR_DEBUG = "\033[34m";
const std::string Log::COLOR_INFO = "\033[32m";
const std::string Log::COLOR_WARNING = "\033[33m";
const std::string Log::COLOR_ERROR = "\033[31m";
const std::string Log::COLOR_CRITICAL = "\033[1;5;31m";
const std::string Log::COLOR_RESET = "\033[0m";

Log& Log::getInstance() {
    static Log log_;
    std::cout << "creating log instance..." << std::endl;
    log_.addLog(LogLevel::Info, "LOG", "Logger init...");
    return log_;
}

struct Log::Impl {
    Impl() : log_config_(LogConfig::getConfig()) {}

    class LogWriter {
    public:
        std::future<bool> operator()(Log& log);
    };

    const LogConfig& log_config_;
    std::atomic<bool> flag_;
    LogQueue buffer_;
    std::vector<std::future<bool>> pool_;
    LogFileManager manager_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<int> ready_count{0};
    std::condition_variable ready_cv;
    std::mutex ready_mtx;

#ifdef LOG_DEBUG
    void debug(const std::string& msg) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
#endif
    static void output_log_(const std::string& color, const std::string& level_str, 
                            const std::string& module, const std::string& message, 
                            bool is_error = false);
};

std::future<bool> Log::Impl::LogWriter::operator()(Log& log) {
    std::packaged_task<bool(Log&)> task([](Log& log) -> bool {
        auto& impl = *log.pimpl_;
        
#ifdef LOG_DEBUG
        impl.debug("LogWriter task started");
#endif

        while (impl.flag_ || !impl.buffer_.empty()) {
            std::unique_lock<std::mutex> lock(impl.mtx_);

            if (!impl.buffer_.empty()) {
                LogEntry entry = impl.buffer_.front_and_pop();
                lock.unlock();
                
#ifdef LOG_DEBUG
                impl.debug("Writing entry: " + entry.message);
#endif

                impl.manager_.writeInFile(entry);
            } else {
                #ifdef LOG_DEBUG
                impl.debug("Worker waiting...");
                #endif
                
                impl.cv_.wait(lock, [&impl] { 
                    return !impl.buffer_.empty() || !impl.flag_; 
                });
            }
        }
        return impl.buffer_.empty();
    });

    auto task_ptr = std::make_shared<decltype(task)>(std::move(task));
    std::thread([task_ptr, &log] { (*task_ptr)(log); }).detach();
    
#ifdef LOG_DEBUG
    log.pimpl_->debug("LogWriter task dispatched");
#endif
    
    return task_ptr->get_future();
}

void Log::Impl::output_log_(const std::string& color, const std::string& level_str, 
                        const std::string& module, const std::string& message, 
                        bool is_error) {
    auto& stream = is_error ? std::cerr : std::cout;
    stream << color << TimeStamp::now() << " [" << level_str << "] " << COLOR_RESET
            << "<" << module << "> " << message << std::endl;
}

Log::Log() : pimpl_(std::make_unique<Impl>()) {
    pimpl_->flag_ = true;
    
    if (pimpl_->log_config_.usingThreadpool()) {
        pimpl_->pool_.resize(pimpl_->log_config_.threadNumber());
        for (int i = 0; i < pimpl_->log_config_.threadNumber(); i++) {
            Impl::LogWriter f;
            pimpl_->pool_[i] = f(*this);
#ifdef LOG_DEBUG
            pimpl_->debug("Thread " + std::to_string(i) + " initialized");
#endif
        }
    }
}

void Log::addLog(LogLevel level, std::string module, const std::string& msg) {
    if (!pimpl_->flag_) throw std::runtime_error("[log.cpp::addLog()] Logger is closed!!!");

    if(pimpl_->log_config_.terminal_print()) {
        terminal_log(level, module, msg);
    }

    LogEntry entry(level, module, msg);

    if (pimpl_->log_config_.usingThreadpool()) {
        std::lock_guard<std::mutex> lock(pimpl_->mtx_);
        pimpl_->buffer_.push(entry);
        pimpl_->cv_.notify_one();
        
#ifdef LOG_DEBUG
        pimpl_->debug("Buffer size: " + std::to_string(pimpl_->buffer_.size()));
        pimpl_->debug("Entry added: " + msg);
#endif
    } else {
        pimpl_->manager_.writeInFile(entry);
    }
}

void moshi::Log::terminal_log(LogLevel level, const std::string& module, const std::string& msg) {
    switch (level) {
        case LogLevel::Debug:    Impl::output_log_(COLOR_DEBUG,    "Debug",    module, msg); break;
        case LogLevel::Info:     Impl::output_log_(COLOR_INFO,     "Info",     module, msg); break;
        case LogLevel::Warning:  Impl::output_log_(COLOR_WARNING,  "Warning",  module, msg); break;
        case LogLevel::Error:    Impl::output_log_(COLOR_ERROR,    "Error",    module, msg, true); break;
        case LogLevel::Critical: Impl::output_log_(COLOR_CRITICAL, "Critical", module, msg, true); break;
    }
}

void Log::close() {
    addLog(LogLevel::Info, "LOG", "logger is destoried...");
    // 若是使用了线程池就清理线程池中的所有条目
    if (pimpl_->log_config_.usingThreadpool()) {
        pimpl_->flag_ = false;
        pimpl_->cv_.notify_all();
        for (auto& fut : pimpl_->pool_) 
            fut.wait();
        
#ifdef LOG_DEBUG
        pimpl_->debug("Log closed successfully");
#endif
    }
}

Log::~Log() {
    close();
    std::cout << "log instance was destory!" << std::endl;
}