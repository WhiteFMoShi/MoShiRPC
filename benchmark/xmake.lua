target("log_bench")
    set_kind("binary")
    add_includedirs("../include/moshirpc")
    add_files("mos_log_bench.cpp")
    add_deps("common_module")

target("timer_bench")
    set_kind("binary")
    add_includedirs("../include/moshirpc")
    add_files("timer_bench.cpp")
    add_deps("common_module")

target("threadpool_bench")
    set_kind("binary")
    add_includedirs("../include/moshirpc")
    add_files("threadpool_bench.cpp")
    -- Only for linking: use the existing implementation unit.
    add_files("../src/common/thread_pool.cpp")
