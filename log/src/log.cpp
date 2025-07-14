#include <iostream>
#include <mutex>
#include <future>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "log.hpp"
#include "log_config.hpp"
#include "log_file_manager.hpp"
#include "log_queue.hpp"

using namespace MoShi;

// #define LOG_DEBUG

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

Log::Log() : pimpl_(std::make_unique<Impl>()) {
    pimpl_->flag_ = pimpl_->log_config_.usingThreadpool();
    
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
    if (!pimpl_->flag_) throw std::runtime_error("Logger is closed!!!");

    if (pimpl_->log_config_.usingThreadpool()) {
        LogEntry entry(level, module, msg);
        std::lock_guard<std::mutex> lock(pimpl_->mtx_);
        pimpl_->buffer_.push(entry);
        pimpl_->cv_.notify_all();
        
#ifdef LOG_DEBUG
        pimpl_->debug("Buffer size: " + std::to_string(pimpl_->buffer_.size()));
        pimpl_->debug("Entry added: " + msg);
#endif
    } else {
        LogEntry entry(level, module, msg);
        pimpl_->manager_.writeInFile(entry);
    }
}

void Log::close() {
    addLog(LogLevel::Info, "LOG", "logger is destoried...");
    // 若是使用了线程池就清理线程池中的所有条目
    if (pimpl_->log_config_.usingThreadpool()) {
        pimpl_->flag_ = false;
        pimpl_->cv_.notify_all();
        for (auto& fut : pimpl_->pool_) fut.wait();
        
#ifdef LOG_DEBUG
        pimpl_->debug("Log closed successfully");
#endif
    }
}

Log::~Log() {
    close();
    std::cout << "log instance was destory!" << std::endl;
}