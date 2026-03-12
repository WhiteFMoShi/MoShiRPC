#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include "log.hpp"

using namespace moshi;

// 测试参数
const int NUM_THREADS = 4;
const int LOGS_PER_THREAD = 10000;
const int TOTAL_LOGS = NUM_THREADS * LOGS_PER_THREAD;

// 原子计数器
std::atomic<int> counter{0};

void benchmark_single_thread() {
    std::cout << "=== 单线程性能测试 ===" << std::endl;
    
    auto& logger = Log::get_instance();
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TOTAL_LOGS; ++i) {
        logger.add_log(LogLevel::Info, "BENCHMARK", 
                      "Log entry " + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "总日志数: " << TOTAL_LOGS << std::endl;
    std::cout << "耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "吞吐量: " << (TOTAL_LOGS * 1000.0 / duration.count()) << " logs/s" << std::endl;
    std::cout << std::endl;
}

void benchmark_multi_thread() {
    std::cout << "=== 多线程性能测试 ===" << std::endl;
    
    auto& logger = Log::get_instance();
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&logger, i]() {
            auto thread_start = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < LOGS_PER_THREAD; ++j) {
                logger.add_log(LogLevel::Info, "BENCHMARK", 
                              "Log entry " + std::to_string(counter++));
            }
            auto thread_end = std::chrono::high_resolution_clock::now();
            auto thread_duration = std::chrono::duration_cast<std::chrono::milliseconds>(thread_end - thread_start);
            std::cout << "线程 " << i << " 完成, 耗时: " << thread_duration.count() << " ms" << std::endl;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "总日志数: " << TOTAL_LOGS << std::endl;
    std::cout << "线程数: " << NUM_THREADS << std::endl;
    std::cout << "总耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "吞吐量: " << (TOTAL_LOGS * 1000.0 / duration.count()) << " logs/s" << std::endl;
    std::cout << std::endl;
}

void benchmark_move_semantics() {
    std::cout << "=== 移动语义优化测试 ===" << std::endl;
    
    auto& logger = Log::get_instance();
    auto start = std::chrono::high_resolution_clock::now();
    
    // 测试移动语义优化：使用临时字符串
    for (int i = 0; i < TOTAL_LOGS / 10; ++i) {
        // 使用临时字符串，应该会触发移动语义优化
        std::string module = "MODULE_" + std::to_string(i % 10);
        std::string msg = "Message with number: " + std::to_string(i);
        
        logger.add_log(LogLevel::Debug, std::move(module), std::move(msg));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "移动语义测试日志数: " << TOTAL_LOGS / 10 << std::endl;
    std::cout << "耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "吞吐量: " << (TOTAL_LOGS * 100.0 / duration.count()) << " logs/s" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "开始log库性能基准测试..." << std::endl;
    std::cout << "测试配置:" << std::endl;
    std::cout << "- 线程数: " << NUM_THREADS << std::endl;
    std::cout << "- 每线程日志数: " << LOGS_PER_THREAD << std::endl;
    std::cout << "- 总日志数: " << TOTAL_LOGS << std::endl;
    std::cout << std::endl;
    
    // 等待log系统初始化
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 运行测试
    benchmark_single_thread();
    
    // 重置计数器
    counter = 0;
    
    benchmark_multi_thread();
    
    benchmark_move_semantics();
    
    std::cout << "性能测试完成!" << std::endl;
    
    // 等待所有日志写入完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}