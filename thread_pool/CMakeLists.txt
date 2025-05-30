cmake_minimum_required(VERSION 3.15)
project(ThreadPool LANGUAGES CXX)

# 全局设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 关键修改：仅添加实际存在的源文件
add_library(thread_pool
    src/thread_pool.cpp  # 仅包含有实现的.cpp文件
    # 注意：safe_queue.hpp 是头文件模板，不应添加到add_library中
)

# 完整包含路径配置
target_include_directories(thread_pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  # 必须添加src目录
    $<INSTALL_INTERFACE:include>
)

# 安装规则
install(TARGETS thread_pool
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(FILES include/thread_pool.hpp DESTINATION include)