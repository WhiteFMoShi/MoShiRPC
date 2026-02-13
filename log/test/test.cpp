#include "log.hpp"
#include <thread>
#include <vector>
#include <chrono>

using namespace moshi;

// 每个线程执行的日志任务
void thread_log_task(Log& logger, const std::string& thread_name, int num_logs) {
    for (int i = 0; i < num_logs; ++i) {
        LogLevel level = static_cast<LogLevel>(i % 5); // 循环使用 Debug -> Critical
        std::string msg = "Thread [" + thread_name + "] log #" + std::to_string(i) +
                          ", level: " + std::to_string(static_cast<int>(level));
        logger.add_log(level, thread_name, msg);

        // 模拟一些工作负载，增加并发交错的可能性
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    Log& logger = Log::get_instance();

    // // 先测试单线程（可选，和你原来的代码一样）
    // logger.addLog(LogLevel::Info, "main", "This is a single-thread info log.");
    // logger.addLog(LogLevel::Debug, "main", "This is a single-thread debug log.");
    // logger.addLog(LogLevel::Error, "main", "This is a single-thread error log.");
    // logger.addLog(LogLevel::Warning, "main", "This is a single-thread warning log.");
    // logger.addLog(LogLevel::Critical, "main", "This is a single-thread critical log.");

    // // 调用一次 terminal_log，测试是否正常
    Log::terminal_log(LogLevel::Info, "main", "This is a terminal log message from main thread.");

    // ========== 多线程测试开始 ==========
    const int kNumThreads = 4;       // 线程数
    const int kLogsPerThread = 20;    // 每个线程打印的日志数

    std::vector<std::thread> threads;

    for (int i = 0; i < kNumThreads; ++i) {
        std::string thread_name = "thread-" + std::to_string(i);
        threads.emplace_back(thread_log_task, std::ref(logger), thread_name, kLogsPerThread);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    // ========== 多线程测试结束 ==========

    return 0;
}