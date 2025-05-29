project(log LANGUAGES CXX)

# 关键修改1：添加所有头文件路径
add_library(log
    src/log.cpp
    src/config/log_config.cpp
    src/entry/log_entry.cpp
    src/entry/format/log_format.cpp
    src/file/log_file_manager.cpp
    src/utils/log_queue.cpp
    src/utils/time_stamp.cpp
)

# 关键修改2：完整包含路径配置
target_include_directories(log PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  # 必须添加src目录
    $<INSTALL_INTERFACE:include>
)

# 关键修改3：显式声明内部依赖关系
target_link_libraries(log PRIVATE
    file
    entry
    utils
    config
)