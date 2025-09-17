#!/usr/bin/env bash

# 检查当前目录下是否存在 cJSON 目录
if [ ! -d "cJSON" ]; then
    # 若不存在 cJSON 目录，则执行 git clone 操作
    git clone git@github.com:DaveGamble/cJSON.git
    if [ $? -eq 0 ]; then
        echo "clone 'cJson' false or directory already existing."
    fi
fi
(cd cJSON && make all) # 在子shell中执行这些指令，这样就不会影响当前所在的目录

if [ $? -eq 0 ]; then
   echo "cJSO:make had do nothing!!!"
fi

if [ ! -d "spdlog" ]; then
    git clone git@github.com:gabime/spdlog.git
    if [ $? -eq 0 ]; then
        echo "clone 'spdlog' false or directory already existing."
    fi
fi

if [ $? -eq 0 ]; then
    echo "spdlog:make had do nothing!!!"
fi