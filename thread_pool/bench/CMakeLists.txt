# 创建可执行文件，直接包含所有需要的源文件
add_executable(example
    example.cpp                  # 主程序文件
    ${CMAKE_SOURCE_DIR}/../src/thread_pool.cpp  # 线程池实现
    ${CMAKE_SOURCE_DIR}/../src/safe_queue.hpp   # 内部头文件依赖
)

# 设置包含路径
target_include_directories(example PRIVATE
    ${CMAKE_SOURCE_DIR}/../include  # thread_pool.hpp
    ${CMAKE_SOURCE_DIR}/../src     # safe_queue.hpp
)

# 添加自动运行目标（make后直接执行）
add_custom_target(run_all
    COMMAND ${CMAKE_COMMAND} -E echo "Running example..."
    COMMAND ./example
    DEPENDS example
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 设置为默认目标（输入make时自动构建并运行）
set_target_properties(run_all PROPERTIES EXCLUDE_FROM_ALL FALSE)