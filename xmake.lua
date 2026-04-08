add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
set_languages("c++17")

on_clean(function()
    os.rm("build")
    os.rm(".xmake")
    os.rm(".vscode")
    os.rm(".cache")
end)

add_requires("gtest 1.17.0")
add_requires("sonic-cpp")
add_requires("spdlog")

target("network_module")
    set_kind("static")
    add_includedirs("include/moshirpc")
    add_files("src/network/*.cpp")

target("common_module")
    set_kind("static")
    add_includedirs("include/moshirpc")
    add_files("src/common/*.cpp")

includes("tests")
includes("benchmark")