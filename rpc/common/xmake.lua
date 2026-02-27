set_rules("plugin.compile_commands.autoupdate")
add_languages("c++17")

on_clean(
    function()
        os.rm("compile_commands.json")
        os.rm(".xmake")
        os.rm(".cache")
        os.rm("build")
    end
)

add_requires("gtest")

target("common")
    set_kind("static")
    add_files("*.cpp")
    add_includedirs(".", { public = true })

target("test_buffer")
    set_default(false)
    set_kind("binary")
    add_files("tests/test_buffer.cpp")
    add_includedirs(".")
    add_deps("common")
    add_packages("gtest")