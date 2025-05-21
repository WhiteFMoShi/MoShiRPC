#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "log.h"
#include "logConfig/logConfig.h"
#include "logEntry/logEntry.h"

Log& Log::getInstance() {
    static Log log_; // 静态局部变量
    std::cout << "Log init succ!!!" << std::endl;
    return log_;
}

Log::Log() : log_config_(LogConfig::getConfig()) {
    if(log_config_.usingThreadpool()) {
        pool_.resize(log_config_.threadNumber());
        for(int i = 0; i < log_config_.threadNumber(); i++) {
            LogWriter f;
            pool_[i] = f(*this);  // 对象在构造函数执行期间就已经存在了
                                        // 虽然此时Log对象还没完全初始化
                                        // 只要f()不会用到Log还没初始化的部分就没问题
        }
    }
}

Log::~Log() {
    if(log_config_.usingThreadpool()) {
        for(int i = 0; i < log_config_.threadNumber(); i++) {
            pool_[i].get();
        }
        std::cout << "DisConstruction succ!!!" << std::endl;
    }
}

void Log::addLog(LogLevel level, std::string module, const std::string& msg) {
    LogEntry entry(level, module, msg);
    buffer_.push(entry);
}

std::future<bool> Log::LogWriter::operator()(Log& log) {
    std::cout << "callabe test succ!!!" << std::endl;
    std::packaged_task<bool(Log&)> task([](Log& log) -> bool {
        while(log.flag_) {
            std::unique_lock<std::mutex> locker(log.mtx_);
            if(!log.buffer_.empty()) {
                LogEntry entry = log.buffer_.front_and_pop();
                locker.unlock();

                // 日志写入（不确定这里要不要再上锁，和filemanager相关的）
                log.file_manager_[entry.date()].write(entry);
            }
            else
                log.cv_.wait(locker); // 阻塞等待通知
        }
        return true;
    });

    std::shared_ptr<decltype(task)> ptr = std::make_shared<decltype(task)>(std::move(task));
    std::thread t([ptr, &log]() { (*ptr)(log); });
    t.detach();

    std::future<bool> ret = ptr->get_future();
    return ret;
}
