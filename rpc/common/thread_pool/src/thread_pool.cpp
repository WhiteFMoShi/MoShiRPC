#include "thread_pool.hpp"

// #define THREADPOOL_DEBUG

void ThreadPool::ThreadWorker::operator() () {
    std::function<void()> f;
    
    bool flag = false;

    // 只要线程池是打开的就会一直运行
    while(pool_.is_opening_) {
        std::unique_lock<std::mutex> locker(pool_.mtx_);
        if(pool_.queue_.empty()) // 此时线程池的任务队列是空的
            // 利用谓词避免虚假唤醒
            pool_.cv_.wait(locker, [this] 
                { return!pool_.is_opening_ ||!pool_.queue_.empty(); });
        flag = pool_.queue_.pop(f);

        locker.unlock(); // 不解锁的话，会出现死锁（因为是while）

        if(flag) // 如果成功获取到了任务
            f(); // 解锁后执行
    }
}

void ThreadPool::f_init_() {
#ifdef THREADPOOL_DEBUG
    std::cout << "ThreadPool status: " << std::endl;
    std::cout << "\tThread Number: " << threads_.size() << std::endl;
    std::cout << "\topen status: " << is_opening_ << std::endl;
#endif
    if(is_opening_) {
        for(int i = 0; i < static_cast<int>(threads_.size()); i++) {
                threads_[i] = std::thread(ThreadWorker(*this, i));
        }
    }
}

void ThreadPool::f_turn_off_() {
    if(!is_opening_)
        return;
    is_opening_ = false;
    cv_.notify_all();
    for(int i = 0; i < static_cast<int>(threads_.size()); i++) {
        if(threads_[i].joinable())
            threads_[i].join();
    }
}