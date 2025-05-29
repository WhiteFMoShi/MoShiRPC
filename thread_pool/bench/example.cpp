#include <iostream>

#include "../thread_pool.hpp"

int task(int a, int b) {
    return a + b;
}

int main() {
    ThreadPool pool;
    auto ret = pool.submit(task, 1, 2); // ret == future<int>

    std::cout << ret.get() << std::endl;
}