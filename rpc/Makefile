include ../config.mk

SRC_FILES := $(shell find . -name '*.cpp')
INCLUDE_FILES := $(shell find . -path '*/include/*.hpp')

# 头文件路径处理
INCLUDE_DIRS := $(sort $(dir $(INCLUDE_FILES)))
_LOCAL_CXXFLAGS := $(CXXFLAGS) $(addprefix -I,$(INCLUDE_DIRS))

# 对象文件定义
OBJ_FILES := $(SRC_FILES:.cpp=.o)

# 编译规则
%.o: %.cpp
	echo $(_LOCAL_CXXFLAGS)
	$(CXX) $(_LOCAL_CXXFLAGS) -c $< -o $@

# 默认目标
all: $(OBJ_FILES)

# 清理
clean:
	rm -f $(OBJ_FILES)

.PHONY: all clean