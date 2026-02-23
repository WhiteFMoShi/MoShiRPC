set_rules("plugin.compile_commands.autoupdate")

add_requires("gtest")
add_languages("c++17")

target("buffer")
    set_kind("static")
    add_files("buffer/*.cpp")
    add_includedirs("buffer")

target("test_buffer")
    set_kind("binary")
    add_files("test/test_buffer.cpp")
    add_includedirs("buffer")
    add_deps("buffer")
    add_packages("gtest")