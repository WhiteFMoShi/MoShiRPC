# 编译器和编译选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# 头文件包含路径
INCLUDES = -I../include -I../src -I/home/xfy/module/SDK

# 源文件列表
SRC_FILES = ../src/log.cpp \
            ../src/config/log_config.cpp \
            ../src/entry/log_entry.cpp \
            ../src/entry/format/log_format.cpp \
            ../src/file/log_file_manager.cpp \
            ../src/utils/time_stamp.cpp \
            ../src/utils/log_queue.cpp \
            bench.cpp

# 目标文件列表
OBJ_FILES = $(SRC_FILES:.cpp=.o)

# 可执行文件名称
EXECUTABLE = bench

# 默认目标
all: $(EXECUTABLE)

# 生成可执行文件
$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -rf $(OBJ_FILES) $(EXECUTABLE) Log conf link_test

LIB_DIR := ..
LIBS := -L$(LIB_DIR) -llog  # 关键！-L 指定路径
link_test:
	$(CXX) $(CXXFLAGS) -I../include bench.cpp $(LIBS) -o $@
	./$@

# 构建后自动运行测试
test: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run