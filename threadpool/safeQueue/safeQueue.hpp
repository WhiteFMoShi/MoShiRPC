#pragma once

#include <cstddef>
#include <queue>
#include <mutex>

template <class T>
class SafeQueue {
public:
    void push(T&& val);
    void push(const T& val);
    
    bool pop(T& obj);
    
    std::size_t size() { return q_.size(); }
    bool empty() { return size() == 0; }
private:
    std::queue<T> q_;
    std::mutex mtx_;
};

template <class T>
void SafeQueue<T>::push(T&& val) {
    std::lock_guard<std::mutex> locker(mtx_);
    q_.push(std::move(val));
}

template <class T>
void SafeQueue<T>::push(const T& val) {
    std::lock_guard<std::mutex> locker(mtx_);
    q_.push(val);
}

template <class T>
bool SafeQueue<T>::pop(T& obj) {
    std::lock_guard<std::mutex> locker(mtx_);
    if(q_.empty())
        return false;
    obj = q_.front();
    q_.pop();
    return true;
}