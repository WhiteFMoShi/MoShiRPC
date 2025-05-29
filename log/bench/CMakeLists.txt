cmake_minimum_required(VERSION 3.10)
project(LogBench LANGUAGES CXX)

# 设置编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -g)

# 定义所有需要编译的源文件（根据您的文件树更新路径）
set(SRC_FILES
    ../src/log.cpp
    ../src/config/log_config.cpp
    ../src/entry/log_entry.cpp
    ../src/entry/format/log_format.cpp
    ../src/file/log_file_manager.cpp
    ../src/utils/time_stamp.cpp
    ../src/utils/log_queue.cpp
    bench.cpp  # 当前目录的测试文件
)

# 直接生成可执行文件
add_executable(bench ${SRC_FILES})

# 设置头文件包含路径（关键！）
target_include_directories(bench PRIVATE
    ../include  # log.hpp 所在目录
    ../src     # 所有内部头文件目录
)

# 可选：构建后自动运行测试
add_custom_command(TARGET bench POST_BUILD
    COMMAND ./bench
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running benchmark..."
)