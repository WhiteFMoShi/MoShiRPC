#include <chrono>
#include <atomic>
#include <future>
#include <unistd.h>
#include <vector>
#include <iostream>

#include "log.hpp"

void test_log_performance() {
    Log& logger = Log::getInstance();
    constexpr int PRODUCER_THREADS = 4; // 生产者线程数量
    constexpr int LOGS_PER_THREAD = 500000;
    std::atomic<int> counter{0};

    std::cout << "生产者线程数: " << PRODUCER_THREADS << " , 每个线程产生 " << LOGS_PER_THREAD << " 条log" << std::endl;
    // 记录每个线程的耗时，精确到毫秒
    std::vector<std::chrono::milliseconds> thread_durations(PRODUCER_THREADS);

    // 准备阶段
    auto start = std::chrono::high_resolution_clock::now();

    // 创建生产者线程
    std::vector<std::future<void>> producers;
    for (int i = 0; i < PRODUCER_THREADS; ++i) {
        producers.push_back(std::async(std::launch::async, [&, i] {
            auto thread_start = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < LOGS_PER_THREAD; ++j) {
                logger.addLog(LogLevel::Info, "TEST", 
                    "Log entry " + std::to_string(counter++));
            }
            auto thread_end = std::chrono::high_resolution_clock::now();
            thread_durations[i] = std::chrono::duration_cast<std::chrono::milliseconds>(thread_end - thread_start);
        }));
    }

    // 等待所有生产者完成
    for (auto& producer : producers) {
        producer.wait();
    }

    // 结束阶段
    logger.close();

    auto end = std::chrono::high_resolution_clock::now();

    // 统计结果
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "总日志量: " << PRODUCER_THREADS * LOGS_PER_THREAD << "\n"
              << "总耗时: " << total_duration.count() << " ms\n"
              << "QPS: " << (PRODUCER_THREADS * LOGS_PER_THREAD * 1000) / total_duration.count() 
              << " 条/秒\n";

    // 输出每个线程的耗时
    for (int i = 0; i < PRODUCER_THREADS; ++i) {
        std::cout << "线程 " << i << " addLog花费时间: " << thread_durations[i].count() << " ms\n";
    }
}

int main() {
    test_log_performance();
    return 0;
}