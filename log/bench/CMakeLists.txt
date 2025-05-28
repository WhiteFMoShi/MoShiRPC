# 设置 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(LogBench)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 开启编译选项，对应 Makefile 中的 -Wall -g
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

# 定义源文件列表
set(DIR "/home/xfy/module/log")
set(SRC
    ${DIR}/bench/bench.cpp
    ${DIR}/logConfig/logConfig.cpp
    ${DIR}/logEntry/logFormat/logFormat.cpp
    ${DIR}/logFileManager/logFileManager.cpp
    ${DIR}/utils/timeStamp.cpp
    ${DIR}/logEntry/logEntry.cpp
    ${DIR}/log.cpp
    ${DIR}/utils/logQueue.cpp
)

# 添加可执行文件
add_executable(bench ${SRC})

# 如果你需要运行生成的可执行文件，可以在构建后添加以下命令
add_custom_command(TARGET bench POST_BUILD
    COMMAND ./bench
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running bench executable"
)

# 清理目标，对应 Makefile 中的 clean 规则
add_custom_target(clean_log_bench
    COMMAND rm -rf Log conf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Cleaning Log and conf files"
)