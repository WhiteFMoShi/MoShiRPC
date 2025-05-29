# 要求的 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(ThreadPoolBench)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加编译选项，相当于 Makefile 中的 -Wall -Wextra
add_compile_options(-Wall -Wextra)

# 源文件列表
set(SRCS example.cpp ../threadPool.cpp)

# 可执行文件
set(OBJ example)

# 设置可执行文件输出目录为项目根目录的 build 目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)

# 生成可执行文件
add_executable(${OBJ} ${SRCS})

# 在可执行文件构建完成后立即运行
add_custom_command(TARGET ${OBJ} POST_BUILD
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/${OBJ}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)

# 定义清理目标，类似于 Makefile 中的 clean 目标
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove -f ${EXECUTABLE_OUTPUT_PATH}/${OBJ}
    COMMAND ${CMAKE_COMMAND} -E remove -f ${EXECUTABLE_OUTPUT_PATH}/*.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f ${EXECUTABLE_OUTPUT_PATH}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXECUTABLE_OUTPUT_PATH}/CMakeFiles
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)