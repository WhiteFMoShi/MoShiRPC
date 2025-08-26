include ../../config.mk

# 源文件和目标文件
SRCS = $(shell find src -name *.cpp)
OBJS = $(SRCS:.cpp=.o)

# 测试程序
TEST_SRCS = $(shell find test -name *.cpp)
TEST_BINS = $(TEST_SRCS:.cpp=.out)

LOG_ := conf Logfile
LOG_FILES := $(addprefix test/, $(LOG_))

LDFLAGS := -L$(PROJECT_ROOT)/rpc/network -lnetwork -L$(PROJECT_ROOT)/log -llog

LIB_NAME := $(notdir $(CURDIR))# 获取当前的目录名作为lib文件名

# 默认目标：编译静态库和测试程序
all: lib$(LIB_NAME).a $(TEST_BINS)

static: lib$(LIB_NAME).a

# 静态库规则
lib$(LIB_NAME).a: $(OBJS)
	ar rcs $@ $^

# 测试程序规则（每个 .cpp 生成一个可执行文件）
test/%.out: test/%.cpp lib$(LIB_NAME).a
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# 通用编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

# 清理
clean:
	rm -rf $(OBJS) lib$(LIB_NAME).a $(TEST_BINS) $(LOG_FILES)

.PHONY: all clean