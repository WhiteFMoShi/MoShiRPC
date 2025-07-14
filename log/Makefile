include ../config.mk

TARGETNAME := liblog# 库文件名称
STATIC := a

SRC_DIR := src
SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES := $(SRC_FILES:.cpp=.o)

.PHONY: all clean static

all: static

static: $(TARGETNAME).$(STATIC)

# 静态库打包规则
$(TARGETNAME).$(STATIC): $(OBJ_FILES)
	$(AR) rcs $@ $^

# 通用编译规则：将 .cpp 编译为 .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) $(TARGETNAME).$(STATIC)