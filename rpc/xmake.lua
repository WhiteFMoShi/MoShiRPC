---------- module global ----------
set_version("1.0.0")

add_rules("plugin.compile_commands.autoupdate")
add_rules("mode.debug", "mode.release", "mode.check")
add_languages("c++17") -- gtest不能在低于c++17使用

on_clean(
    function()
        os.rm("build")
        os.rm(".xmake")
        os.rm(".cache")
        os.rm("compile_commands.json")
    end
)

-- 依赖库配置
add_requires("spdlog 1.16.*", 
            "cjson 1.7.*", 
            "protobuf-cpp 33.2",
            "gtest 1.17.*")

includes("network")
includes("protocol")
includes("common")