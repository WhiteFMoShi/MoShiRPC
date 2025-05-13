#include <iostream>
#include <thread>
#include <vector>

#include "safeQueue.h"
// 假设 SafeQueue 的定义在前面已经给出
// 这里省略 SafeQueue 的定义代码

int main() {
    SafeQueue<int> safeQueue;
    const int numPushThreads = 5;
    const int numPopThreads = 5;
    const int numElementsPerThread = 1000;

    // 定义 push 线程的工作函数
    auto pushTask = [&safeQueue, numElementsPerThread]() {
        for (int i = 0; i < numElementsPerThread; ++i) {
            safeQueue.push(i);
        }
    };

    // 定义 pop 线程的工作函数
    auto popTask = [&safeQueue, numElementsPerThread]() {
        int obj;
        for (int i = 0; i < numElementsPerThread; ++i) {
            while (!safeQueue.pop(obj)) {
                // 如果队列暂时为空，稍微等待一下再尝试
                std::this_thread::yield();
            }
        }
    };

    // 创建并启动 push 线程
    std::vector<std::thread> pushThreads;
    for (int i = 0; i < numPushThreads; ++i) {
        pushThreads.emplace_back(pushTask);
    }

    // 创建并启动 pop 线程
    std::vector<std::thread> popThreads;
    for (int i = 0; i < numPopThreads; ++i) {
        popThreads.emplace_back(popTask);
    }

    // 等待所有 push 线程完成
    for (auto& thread : pushThreads) {
        thread.join();
    }

    // 等待所有 pop 线程完成
    for (auto& thread : popThreads) {
        thread.join();
    }

    // 检查队列是否为空
    if (safeQueue.empty()) {
        std::cout << "Queue is empty as expected. Thread safety test passed!" << std::endl;
    } else {
        std::cout << "Queue is not empty! Thread safety test failed." << std::endl;
    }

    return 0;
}