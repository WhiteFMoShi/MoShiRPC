add_rules("plugin.compile_commands.autoupdate")

target("protocol")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { interface = true })
