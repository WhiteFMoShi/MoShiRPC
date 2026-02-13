set_project("moshirpc-log")
set_version("1.0.0")

set_rules("plugin.compile_commands.autoupdate")
add_rules("mode.debug", "mode.release", "mode.check")

add_requires("gtest", "cjson") -- 库

-- 设置xmake的构建文件夹
set_config("buildir", "build")

-- 自动清理，类似make clean
on_clean(
    function()
        os.rm("$(buildir)")
        os.rm(".xmake")
        os.rm(".cache")
        os.rm("compile_commands.json")
    end
)

set_languages("c++17") -- 使用的std::any是C++17提出的

target("moshi_log")
    set_kind("static")
    add_packages("cjson")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })

-- 测试目标（如果需要）
target("log_bench")
    set_default(false)
    set_kind("binary")
    add_files("bench/bench.cpp")
    add_includedirs("include")
    add_deps("log")
    add_packages("gtest")