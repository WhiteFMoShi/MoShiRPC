
set_project("moshirpc-log")
set_version("1.0.0")

set_rules("plugin.compile_commands.autoupdate")
set_rules("mode.debug", "mode.release", "mode.check")

-- 设置语言标准
set_languages("c++17")

-- 添加警告选项
add_cxxflags("-Wall", "-Wextra", "-Wpedantic")

-- 根据模式设置不同的编译选项
if is_mode("debug") then
    add_cxxflags("-g", "-O0")
    add_defines("DEBUG")
elseif is_mode("release") then
    add_cxxflags("-O2", "-DNDEBUG")
end

target("log")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })

-- 测试目标（如果需要）
target("log_test")
    set_default(false)
    set_kind("binary")
    add_files("bench/bench.cpp")
    add_deps("log")
    add_includedirs("include")
    