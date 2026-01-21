set_rules("plugin.compile_commands.autoupdate")
add_rules("mode.debug", "mode.release", "mode.check")

on_config("buildir", "build")
on_clean(
    function()
        os.rm("$(buildir)")
        os.rm(".xmake")
        os.rm(".cache")
        os.rm("compile_commands.json")
    end
)

target("network") -- 导出网络库
    set_kind("static")
    add_includedirs("include", { public = true })
    add_files("src/*.cpp")

target("test_event_loop")
    set_kind("binary")
    set_default(false)
    add_files("test/test_event_loop.cpp")
    add_deps("network")
    add_packages("gtest")

target("test_socket")
    set_kind("binary")
    set_default(false)
    add_files("test/test_socket.cpp")
    add_deps("network")

target("test")
    set_kind("binary")
    set_default(false)
    add_files("test/total_test.cpp")
    add_packages("gtest")
    add_links("gtest_main")
    add_deps("network")