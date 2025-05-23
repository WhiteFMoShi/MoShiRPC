#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "log.h"
#include "logConfig/logConfig.h"
#include "logEntry/logEntry.h"

// #define LOG_DEBUG

Log& Log::getInstance() {
    static Log log_; // 静态局部变量
    std::cout << "Log object is initialized successfully!!!" << std::endl;
    return log_;
}

Log::Log() : log_config_(LogConfig::getConfig()) {
    flag_ = log_config_.usingThreadpool();
    if(log_config_.usingThreadpool()) {
        pool_.resize(log_config_.threadNumber());
        for(int i = 0; i < log_config_.threadNumber(); i++) {
            LogWriter f;
            pool_[i] = f(*this);  // 对象在构造函数执行期间就已经存在了
                                        // 虽然此时Log对象还没完全初始化
                                        // 只要f()不会用到Log还没初始化的部分就没问题
            std::cout << "Thread " << i << " initial!!!" << std::endl;
        }
    }
}

void Log::close() {
    // sleep(1); // 等最后一个日志写入进来
    if(log_config_.usingThreadpool()) {
        flag_ = false; // 关闭
        cv_.notify_all(); // 打破阻塞状态
        if(log_config_.usingThreadpool()) {
            for(int i = 0; i < log_config_.threadNumber(); i++) {
                pool_[i].get();
            }
            std::cout << "Log Object is destructed successfully!!!" << std::endl;
        }
    }
    else {
        
    }
}

void Log::addLog(LogLevel level, std::string module, const std::string& msg) {
    if(flag_ == false) {
        throw std::runtime_error("Logger is closed!!!");
    }
    if(log_config_.usingThreadpool()) {
        LogEntry entry(level, module, msg);
        std::lock_guard<std::mutex> locker(mtx_);
        buffer_.push(entry);
        cv_.notify_all(); // 通知取货

#ifdef LOG_DEBUG
        std::cout << "buffer size: " << buffer_.size() << std::endl;
        std::cout << "Entry add succ!!!" << std::endl;
#endif
    }
    else {
        LogEntry entry(level, module, msg);
        manager_.writeInFile(entry);
    }
}

std::future<bool> Log::LogWriter::operator()(Log& log) {
    std::packaged_task<bool(Log&)> task([](Log& log) -> bool {
#ifdef LOG_DEBUG
        std::cout << "callabe test succ!!!" << std::endl;
#endif
        // 线程启动后立即递增ready_count
        // {
        //     std::lock_guard<std::mutex> ready_lock(log.ready_mtx);
        //     log.ready_count++;
        //     log.ready_cv.notify_all();
        // }

        while(log.flag_ || !log.buffer_.empty()) {
            std::unique_lock<std::mutex> locker(log.mtx_);

            if(!log.buffer_.empty()) {
                LogEntry entry = log.buffer_.front_and_pop();
                locker.unlock();

                // 日志写入（不确定这里要不要再上锁，和filemanager相关的）
                log.manager_.writeInFile(entry);
            }
            else {
                // std::cout << "waiting" << std::endl;
                log.cv_.wait(locker, [&log]() { 
                    return !log.buffer_.empty() || !log.flag_; 
                });
                // std::cout << "working" << std::endl;
            }
        }
        return log.buffer_.empty();
    });

    std::shared_ptr<decltype(task)> ptr = std::make_shared<decltype(task)>(std::move(task));
    std::thread t([ptr, &log]() { (*ptr)(log); });
    t.detach();

    std::future<bool> ret = ptr->get_future();
    return ret;
}
