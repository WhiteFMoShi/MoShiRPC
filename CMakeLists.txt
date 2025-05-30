cmake_minimum_required(VERSION 3.12)
project(ModuleProject LANGUAGES CXX)

# 关键配置：生成Clangd所需的编译命令数据库
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable for IDE tools" FORCE)

# 设置C++标准（根据项目需求调整）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义子模块列表（根据实际需要增减）
set(SUB_MODULES 
    log
    thread_pool
    sql_conn_pool
)

# 递归包含所有子模块
foreach(MODULE ${SUB_MODULES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}/CMakeLists.txt)
        message(STATUS "Configuring submodule: ${MODULE}")
        add_subdirectory(${MODULE})
    else()
        message(WARNING "Submodule ${MODULE} has no CMakeLists.txt")
    endif()
endforeach()

# 可选：创建聚合目标方便整体构建
add_custom_target(all_modules DEPENDS ${SUB_MODULES})

# 可选：将编译数据库软链接到项目根目录（方便Clangd查找）
if(UNIX AND NOT EXISTS "${CMAKE_SOURCE_DIR}/compile_commands.json")
    execute_process(
        COMMAND ln -s "${CMAKE_BINARY_DIR}/compile_commands.json" 
                      "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()