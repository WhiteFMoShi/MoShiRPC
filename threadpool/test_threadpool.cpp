#include "threadPool.h"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

std::atomic<int> counter(0);

// 生成随机整数的函数
int randomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// 模拟具有不同执行时间的任务
void task(int id) {
    // 随机生成 100 到 1000 毫秒的执行时间
    int sleepTime = randomInt(100, 1000);
    std::cout << "Thread " << std::this_thread::get_id() << " executing task "
              << id << ", will sleep for " << sleepTime << " ms" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    counter.fetch_add(1, std::memory_order_relaxed);
}

int main() {
    const int numThreads = 4;
    const int numTasks = 50;
    ThreadPool pool(numThreads);

    // 1. 批量提交任务并统计
    counter = 0;
    std::vector<std::future<void>> results;
    for (int i = 0; i < numTasks; ++i) {
        results.push_back(pool.submit(task, i));
    }
    for (auto &fut : results) {
        fut.get();
    }
    std::cout << "Counter: " << counter.load() << " (should be " << numTasks
              << ")" << std::endl;

    // 2. 测试线程池的启动和关闭流程
    bool exceptionCaught = false;
    try {
        pool.submit(task, 0).get(); // 线程池关闭后提交任务应异常或主线程执行
    } catch (...) {
        exceptionCaught = true;
        std::cout << "Exception caught as expected after pool shutdown."
                  << std::endl;
    }
    if (!exceptionCaught) {
        std::cout << "No exception after shutdown, check if main thread executed "
                     "the task."
                  << std::endl;
    }

    // 3. 多次启动关闭健壮性测试
    ThreadPool pool2(numThreads);
    counter = 0;
    std::vector<std::future<void>> results2;
    for (int i = 0; i < numTasks; ++i) {
        results2.push_back(pool2.submit(task, i));
    }
    for (auto &fut : results2) {
        fut.get();
    }
    std::cout << "Counter after restart: " << counter.load() << " (should be "
              << numTasks << ")" << std::endl;

    // 4. 主线程直接执行任务
    int mainThreadTaskCount = 0;
    auto fut = pool2.submit([&]() {
        std::cout << "Main thread executing task" << std::endl;
        ++mainThreadTaskCount;
    });
    fut.get();
    std::cout << "Main thread task count: " << mainThreadTaskCount
              << " (should be 1)" << std::endl;

    return 0;
}