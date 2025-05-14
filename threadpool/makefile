# 在路径拼接的时候，会莫名其妙出现空格，使用strip消除一下
# strip是makefile的一个内置函数，去除字符串开头和结尾的空白字符（包括空格、制表符和换行符）
WORKSPACE := $(strip $(shell pwd))

SAFEQUEUE_DIR := $(strip $(WORKSPACE))/safeQueue
THREADPOOL_DIR := $(strip $(WORKSPACE))

OBJ_DIR_NAME := Debug

# 编译器和编译选项
CXX := g++
CXXFLAGS := -std=c++17 -Wall

# @用于在执行make的时候不会在命令行中打印本条指令
# mkdir中的-p选项，用于在路径目录不存在时，递归创建目录

# g++ -c选项表示只进行编译
# 由于safeQueue的实现只有.h文件而没有.cpp文件，因此不需要将其编译为.o文件
# queue:
# 	g++ -c $(SAFEQUEUE_DIR)/safeQueue.h -o $(SAFEQUEUE_DIR)/Debug/safeQueue.o

# 使用 && 只有目录创建成功，才会继续进行编译
threadpool: threadPool.cpp
	@mkdir -p ./$(OBJ_DIR_NAME) && \
	$(CXX) $(CXXFLAGS) -c threadPool.cpp -o $(OBJ_DIR_NAME)/threadPool.o

test_threadpool: threadpool 
	$(CXX) $(CXXFLAGS) -o $(OBJ_DIR_NAME)/test_threadpool test_threadpool.cpp $(OBJ_DIR_NAME)/threadPool.o
	$(OBJ_DIR_NAME)/test_threadpool

# 在规则后加上需要的依赖项是个好习惯（加上后依赖关系会进行相关的检查，保证规则使用的文件是最新的）
test_queue: $(SAFEQUEUE_DIR)/test.cpp $(SAFEQUEUE_DIR)/safeQueue.h
	@echo $(WORKSPACE)
	@mkdir -p $(SAFEQUEUE_DIR)/$(OBJ_DIR_NAME) && \
	$(CXX) $(CXXFLAGS) -o $(SAFEQUEUE_DIR)/$(OBJ_DIR_NAME)/test_queue $(SAFEQUEUE_DIR)/test.cpp

	$(SAFEQUEUE_DIR)/$(OBJ_DIR_NAME)/test_queue


.PHONY: clean
clean:
	rm -rf **/$(OBJ_DIR_NAME) && \
	rm -rf $(OBJ_DIR_NAME)