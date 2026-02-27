---------- module global ----------
set_languages("c++17")
set_version("1.0.0")
set_rules("mode.release", "mode.debug")
set_rules("plugin.compile_commands.autoupdate")

set_warnings("all", "error")

-- 依赖库配置
add_requires("spdlog 1.16.*", 
            "cjson 1.7.*", 
            "protobuf-cpp 33.2",
            "gtest 1.17.*")

-- 在homebrew中找package(第三方仓库中的包)
-- 需要使用vpn下载，不然没啥用
-- add_requires("homebrew::zookeeper")

includes("network",
        "protocol",
        "common")
