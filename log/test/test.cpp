#include "log.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

using namespace moshi;

// 辅助函数：测试所有日志级别
void test_all_levels(Log& logger, const std::string& module) {
    logger.addLog(LogLevel::Debug, module, "This is a DEBUG message");
    logger.addLog(LogLevel::Info, module, "This is an INFO message");
    logger.addLog(LogLevel::Warning, module, "This is a WARNING message");
    logger.addLog(LogLevel::Error, module, "This is an ERROR message");
    logger.addLog(LogLevel::Critical, module, "This is a CRITICAL message");
}

// 线程任务函数
void thread_log_task(Log& logger, const std::string& thread_name, int num_logs) {
    for (int i = 0; i < num_logs; ++i) {
        LogLevel level = static_cast<LogLevel>(i % 5);
        std::string msg = "Message #" + std::to_string(i);
        logger.addLog(level, thread_name, msg);
        
        // 随机休眠 0-2ms，增加并发随机性
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 3));
    }
}

int main() {
    std::cout << "=== Log System Test Start ===" << std::endl;
    
    {  // 使用作用域控制日志系统的生命周期
        // 获取日志实例
        Log& logger = Log::getInstance();
        
        // 1. 测试配置
        std::cout << "\n[Test 1] Testing Configuration..." << std::endl;
        logger.configure(true,                    // 异步模式
                        4,                        // 4个线程
                        "test_logs",             // 自定义日志目录
                        true);                   // 打印到终端
        
        // 2. 测试基本日志功能
        std::cout << "\n[Test 2] Testing Basic Logging..." << std::endl;
        test_all_levels(logger, "BasicTest");

        // 3. 测试仅终端输出
        std::cout << "\n[Test 3] Testing Terminal-only Logging..." << std::endl;
        Log::terminal_log(LogLevel::Info, "TerminalTest", "This message should only appear in terminal");
        
        // 4. 测试异步多线程日志
        std::cout << "\n[Test 4] Testing Multi-threaded Logging..." << std::endl;
        const int kNumThreads = 4;
        const int kLogsPerThread = 200;
        std::vector<std::thread> threads;
        
        // 创建多个线程
        for (int i = 0; i < kNumThreads; ++i) {
            std::string thread_name = "Thread-" + std::to_string(i);
            threads.emplace_back(thread_log_task, std::ref(logger), thread_name, kLogsPerThread);
        }
        
        // 等待所有线程完成
        for (auto& t : threads) {
            t.join();
        }
        
        // 5. 测试配置更改
        std::cout << "\n[Test 5] Testing Configuration Changes..." << std::endl;
        logger.setAsyncMode(false);
        logger.setPrintToTerminal(false);
        test_all_levels(logger, "ConfigChangeTest");
        
        // 确保所有日志都被写入
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 在作用域结束前显式调用 stop
        // logger.stop();
    }  // logger 的析构会在这里触发

    std::cout << "Ending!!!…………" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 给足够时间让资源清理完成
    return 0;
}