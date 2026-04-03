add_rules("plugin.compile_commands.autoupdate")

add_requires("cjson")

target("protocol")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { interface = true })
    add_packages("cjson")
