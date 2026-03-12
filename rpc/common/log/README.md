## Build
使用`make`将会默认编译静态库`liblog.a`

## log.config
默认在**工作目录**的``conf``文件夹中，有以下可供配置的项：
```config
asynchronous: false
thread_number: 4
log_dir_relative_path: /Logfile
terminal_print: true
```

## Example
```cpp
#include <log.hpp>

int main() {
    moshi::Log& log = moshi::Log::getInstance();

    // 向文件添加日志
    log.addLog(moshi::LogLevel::Info, "example", "This is a example.");

    // 直接将日志打印到终端中
    log.terminal_log(moshi::LogLevel::Info, "example", "Print to terminal.");
}
```