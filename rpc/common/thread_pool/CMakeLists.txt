cmake_minimum_required(VERSION 3.10)
project(thread_pool LANGUAGES CXX)

# 创建线程池库
add_library(thread_pool
    src/thread_pool.cpp
    src/safe_queue.hpp
    include/thread_pool.hpp
)

# 配置包含路径 - 确保clangd能找到所有头文件
target_include_directories(thread_pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  # 添加src目录确保内部头文件可见
)
