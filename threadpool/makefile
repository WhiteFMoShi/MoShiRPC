# 在路径拼接的时候，会莫名其妙出现空格，使用strip消除一下
# strip是makefile的一个内置函数，去除字符串开头和结尾的空白字符（包括空格、制表符和换行符）
WORKSPACE := $(strip $(shell pwd))
SAFEQUEUE_DEBUG := $(strip $(WORKSPACE))/safeQueue/Debug

# @用于在执行make的时候不会在命令行中打印本条指令
# mkdir中的-p选项，用于在路径目录不存在时，递归创建目录
test_queue:
	@echo $(WORKSPACE)
	@mkdir -p $(SAFEQUEUE_DEBUG)
	g++ -std=c++17 -o $(SAFEQUEUE_DEBUG)/test_queue ./safeQueue/test.cpp
	$(SAFEQUEUE_DEBUG)/test_queue

# .PHONY: clean