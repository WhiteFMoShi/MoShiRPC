#pragma once
#include "./safeQueue/safeQueue.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(const int& thread_num = 4) : pool_(std::vector<std::thread>(thread_num)), is_opening_(true) {
        f_init_();
    }
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;

    ThreadPool& operator=(const ThreadPool& other) = delete; // 返回值要是引用，主要和语义有关
    ThreadPool& operator=(ThreadPool&& other) = delete;

    // 核心代码（难点）
    template <class T, class... Args> // 可变参数模板
    // future和线程入口函数的返回值绑定
    // 此处的T&&利用的是模板的通用引用特性（根据上下文自动推导为左值引用或右值引用）
    auto submit(T&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
        std::function<decltype(func(args...))> f = std::bind(func, args...);
    }
private:
    // 作为thread的入口函数
    class ThreadWorker {
    public:
        ThreadWorker(ThreadPool& pool, int id) : pool_(pool), id_(id) {}
        
        // 从线程池的任务队列中获取任务并执行
        void operator() ();
    private:
        int id_; // 任务的ID
        ThreadPool& pool_; // 所属的线程池
    };
    
    bool is_opening_;
    std::condition_variable cv_; // 用于获取任务相关的环境变量
    std::mutex mtx_; // 互斥锁
    std::vector<std::thread> pool_; // 线程池
    SafeQueue<std::function<void()>> queue_; // 任务队列

    void f_init_();
};

void ThreadPool::ThreadWorker::operator() () {
    std::function<void()> f;
    
    bool flag = false;

    while(pool_.is_opening_) {
        std::unique_lock<std::mutex> locker(pool_.mtx_);
        if(pool_.pool_.empty()) // 此时线程池的任务队列是空的
            pool_.cv_.wait(locker); // 等待条件变量的通知
        flag = pool_.queue_.pop(f);
        if(flag) // 如果成功获取到了任务
            f();
    }
}

void ThreadPool::f_init_() {
    for(int i = 0; i < pool_.size(); i++) {
        pool_[i] = std::thread(ThreadWorker(*this, i));
    }
}