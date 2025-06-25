
# 检查当前目录下是否存在 cJSON 目录
if [ ! -d "cJSON" ]; then
    # 若不存在 cJSON 目录，则执行 git clone 操作
    git clone git@github.com:DaveGamble/cJSON.git
fi
cd cJSON && make all