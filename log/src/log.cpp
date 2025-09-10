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
    return log_;
}

struct Log::Impl {
    Impl() : log_config_(LogConfig::getInstance()) {}

    class LogWriter {
    public:
        std::future<bool> operator()(Log& log);
    };

    LogConfig& log_config_;  // 改为非const引用
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
        
        while (impl.flag_ || !impl.buffer_.empty()) {
            LogEntry entry;
            {
                std::unique_lock<std::mutex> lock(impl.mtx_);
                
                if (impl.buffer_.empty() && impl.flag_) {
                    impl.cv_.wait(lock, [&impl] { 
                        return !impl.buffer_.empty() || !impl.flag_; 
                    });
                }
                
                if (impl.buffer_.empty() && !impl.flag_) {
                    break;
                }
                
                entry = impl.buffer_.front_and_pop();
                impl.cv_.notify_all();  // 通知等待的线程
            }
            
            // 在锁外写入文件
            impl.manager_.writeInFile(entry);
        }
        return true;
    });

    auto task_ptr = std::make_shared<decltype(task)>(std::move(task));
    std::thread([task_ptr, &log] { (*task_ptr)(log); }).detach();
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
    initLogger();
}

void Log::initLogger() {
    pimpl_->flag_ = true;
    
    if (pimpl_->log_config_.isAsyncMode()) {
        pimpl_->pool_.resize(pimpl_->log_config_.getThreadPoolSize());
        for (int i = 0; i < pimpl_->log_config_.getThreadPoolSize(); i++) {
            Impl::LogWriter f;
            pimpl_->pool_[i] = f(*this);
#ifdef LOG_DEBUG
            pimpl_->debug("Thread " + std::to_string(i) + " initialized");
#endif
        }
    }
    addLog(LogLevel::Info, "log", "Logger initialized...");
}

void Log::configure(bool async, int threadPoolSize, 
                   const std::filesystem::path& logDir, bool printToTerminal) {
    pimpl_->log_config_.configure(async, threadPoolSize, logDir, printToTerminal);
    // 重新初始化logger
    stop();
    initLogger();
}

void Log::setAsyncMode(bool async) {
    pimpl_->log_config_.setAsyncMode(async);
    stop();
    initLogger();
}

void Log::setThreadPoolSize(int size) {
    pimpl_->log_config_.setThreadPoolSize(size);
    stop();
    initLogger();
}

void Log::setPrintToTerminal(bool print) {
    pimpl_->log_config_.setPrintToTerminal(print);
}

void Log::setLogDirectory(const std::filesystem::path& dir) {
    pimpl_->log_config_.setLogDirectory(dir);
}

void Log::addLog(LogLevel level, std::string module, const std::string& msg) {
    if (!pimpl_->flag_) throw std::runtime_error("[log.cpp::addLog()] Logger is closed!!!");

    if(pimpl_->log_config_.isPrintToTerminal()) {
        terminal_log(level, module, msg);
    }

    LogEntry entry(level, module, msg);

    if (pimpl_->log_config_.isAsyncMode()) {
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

void moshi::Log::set_log_file_suffix(const std::string& suffix) {
    // 实现文件后缀设置
    // TODO: 实现该功能
}

void Log::stop() {
    if (!pimpl_->flag_) return;  // 避免重复停止
    
    addLog(LogLevel::Info, "log", "stopping...");
    pimpl_->flag_ = false;
    
    if (pimpl_->log_config_.isAsyncMode()) {
        pimpl_->cv_.notify_all();
        // 等待所有写入操作完成
        std::unique_lock<std::mutex> lock(pimpl_->mtx_);
        while (!pimpl_->buffer_.empty()) {
            pimpl_->cv_.wait(lock);
        }
    }
    
#ifdef LOG_DEBUG
    pimpl_->debug("Log stopped successfully");
#endif
}

Log::~Log() {
    // 先停止所有日志写入
    stop();
    
    // 等待所有异步任务完成
    if (pimpl_->log_config_.isAsyncMode()) {
        for (auto& fut : pimpl_->pool_) {
            if (fut.valid()) {
                fut.wait();
            }
        }
    }
    
    // 清空 pimpl_ 以触发 LogFileManager 的析构
    pimpl_.reset();
    
    std::cout << "log instance was destory!" << std::endl;
}