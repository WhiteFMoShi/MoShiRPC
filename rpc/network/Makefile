# include ../../config.mk

# TARGETNAME := net
# STATIC := a

# SRC_FILES := $(shell find src -name *.cpp)
# OBJ_FILES := $(SRC_FILES:.cpp=.o)

# all: static

# static: $(TARGETNAME).$(STATIC)

# $(TARGETNAME).$(STATIC): $(OBJ_FILES)
# 	$(AR) rcs $@ $^

# .PHONY:	clean static
# clean:
# 	rm -f $(OBJ_FILES) *.a


# 编译器配置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -I../include  # 添加多级头文件路径
LDFLAGS = -L. -lnetwork     # 链接当前目录的 libnetwork.a

# 源文件和目标文件
SRCS = $(shell find src -name *.cpp)
OBJS = $(SRCS:.cpp=.o)

# 测试程序
TEST_SRCS = $(shell find test -name *.cpp)
TEST_BINS = $(TEST_SRCS:.cpp=)

# 默认目标：编译静态库和测试程序
all: libnetwork.a $(TEST_BINS)

# 静态库规则
libnetwork.a: $(OBJS)
	ar rcs $@ $^

# 测试程序规则（每个 .cpp 生成一个可执行文件）
test/%: test/%.cpp libnetwork.a
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# 通用编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) libnetwork.a $(TEST_BINS)

.PHONY: all clean