## 📃任务清单
<font color="red"><b>点击链接跳转查看[TODO List](TODO.md)！！！欢迎issue和pr！！！</b></font>

## 大纲

👉 [简介](#modules)

👉 [Building]()

👉 [Tutorials](#tutorial)

👉 [线程池ThreadPool](#threadpool)

👉 [日志Log](#log)

## 🏅项目简介

## Building
本项目编写于`Ubuntu 22.04`，其它OS运行情况未知，欢迎反馈！！！

首先，将项目`clone`至本地：
```shell
git clone https://github.com/WhiteFMoShi/MoShiRPC
```
若是希望构建开发环境：
```shell
cd MoShiRPC
make third_patry
```
将会自动编译第三方依赖，但不会将依赖库放于合适位置，建议运行：
```shell
make build
```
这将为您处理依赖库的同时，于`build/bin`中生成项目可执行文件。

## 📕Previous Tutorials
#### Tools Tutorials
[CMake官方教程](https://cmake-doc.readthedocs.io/zh-cn/latest/guide/tutorial/index.html)

[Makefile快速教程](https://makefiletutorial.com/#getting-started)

[git commits](https://www.conventionalcommits.org/zh-hans/v1.0.0/)

[Doxygen注释]()

#### Contexts Tutorials
[C++ 网络编程]()

[muduo]()

## 📂模块详细说明

### third_party
<font color="red"><b>所有外部库文件的下载、编译无需手动，在项目根目录下执行`make third_party`即可。</b></font>

使用[cJson](https://github.com/DaveGamble/cJSON)库作为本项目的Json生成、解析，`construction.sh`会完成自动的下载、编译操作。

### Log
一个高效、可配置、简单易用的[日志](Note/日志模块.md)。（预期使用Json对类行为进行配置）

#### 💡Log架构设计
![设计阐述](srceenshot/log_construction_design.png)

#### 🚀性能测试
测试代码：[bench](log/bench/bench.cpp)，实际上我觉得测不太出🤣

|写入方式|QPS|
|:--|:--|
|同步写入|676,589条/秒|
|异步写入(2线程)|325,892条/秒|

1. 同步写入测试（单线程）：
![sync_write](srceenshot/sync_write.png)

2. 异步写入测试（2线程）：
![async_write](srceenshot/async_write.png)
