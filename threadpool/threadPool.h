#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

#include "./safeQueue/safeQueue.h"

class ThreadPool {
public:
    ThreadPool(const int& thread_num = 4, bool status = true) : 
        is_opening_(status), threads_(std::vector<std::thread>(thread_num)) {
        f_init_();
    }
    ~ThreadPool() {
        f_turn_off_();
    }
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;

    ThreadPool& operator=(const ThreadPool& other) = delete; // 返回值要是引用，主要和语义有关
    ThreadPool& operator=(ThreadPool&& other) = delete;

    // 向线程池中提交任务，使用std::future获取该任务的执行情况
    template <class T, class... Args> // 可变参数模板
    // future和线程入口函数的返回值绑定
    // 此处的T&&利用的是模板的通用引用特性（根据上下文自动推导为左值引用或右值引用）
    auto submit(T&& func, Args&& ...args) -> std::future<decltype(func(args...))>;
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
    
    bool is_opening_;
    std::condition_variable cv_; // 用于获取任务相关的环境变量
    std::mutex mtx_; // 互斥锁
    std::vector<std::thread> threads_; // 线程池
    SafeQueue<std::function<void()>> queue_; // 任务队列（执行的都是void())

    void f_init_();
    void f_turn_off_(); // 关闭线程池
};

template <class T, class... Args>
auto ThreadPool::submit(T&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
    // 完美转发用于不必要的开销，同时也是为了保证语义正确
    // 完美转发利用的是通用引用，是属于模板的内容  
    // bind在将函数参数进行完美匹配之后，将函数变为一个无参函数
    using ret_type = decltype(func(args...));
    // 包装后的f还有返回值，但是参数列表已经没了：ret_type()
    std::function<ret_type()> f = std::bind(std::forward<T>(func), std::forward<Args>(args)...);

    // ptr去进行操作
    auto task_ptr = std::make_shared<std::packaged_task<ret_type()>>(f);
    // 使用值捕获，确保不会悬空引用
    auto task = [task_ptr]() {
        (*task_ptr)();
    };

    queue_.push(task);
    cv_.notify_one(); // 唤醒一个休眠的线程
    return task_ptr->get_future();
}


