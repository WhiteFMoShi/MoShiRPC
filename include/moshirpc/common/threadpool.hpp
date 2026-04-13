#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

#include "safe_queue.hpp"

class ThreadPool {
public:
    ThreadPool(const int& thread_num = 4, bool status = true) : 
        is_opening_(status), threads_(std::vector<std::thread>(thread_num)) {
        Init_();
    }
    ~ThreadPool() {
        Stop_();
    }
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;

    ThreadPool& operator=(const ThreadPool& other) = delete; // 返回值要是引用，主要和语义有关
    ThreadPool& operator=(ThreadPool&& other) = delete;

    // 向线程池中提交任务，使用std::future获取该任务的执行情况
    template <class T, class... Args> // 可变参数模板
    auto Submit(T&& func, Args&& ...args) -> std::future<decltype(func(args...))>;
private:
    // 作为thread的入口函数
    class ThreadWorker {
    public:
        ThreadWorker(ThreadPool& pool, int id) : pool_(pool), id_(id) {
            std::cout << "ThreadWorker start, id:" << id << std::endl; 
        }

        // 从线程池的任务队列中获取任务并执行
        void operator() ();
    private:
        ThreadPool& pool_; // 所属的线程池
        int id_; // 任务的ID
    };
    
    std::atomic<bool> is_opening_;
    std::condition_variable cv_; // 用于获取任务相关的环境变量
    std::mutex mtx_; // 互斥锁
    std::vector<std::thread> threads_; // 线程池
    SafeQueue<std::function<void()>> queue_; // 任务队列（执行的都是void())

    void Init_();
    void Stop_(); // 关闭线程池
};

template <class T, class... Args>
auto ThreadPool::Submit(T&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
    using ret_type = decltype(func(args...));
    std::function<ret_type()> f = std::bind(std::forward<T>(func), std::forward<Args>(args)...);

    auto task_ptr = std::make_shared<std::packaged_task<ret_type()>>(f);
    auto task = [task_ptr]() {
        (*task_ptr)();
    };

    queue_.push(task);
    cv_.notify_one(); // 唤醒一个休眠的线程
    return task_ptr->get_future();
}
