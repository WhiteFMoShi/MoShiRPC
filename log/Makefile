CXX := g++

SDK := /home/xfy/module/SDK# 不知道怎么设置自动化寻找SDK文件夹，可能要在外部的shell或者makefile中进行设置，然后导入该makefile中
LIBS := $(SDK)
PROJECT_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
INCLUDE_PATH := -I$(PROJECT_ROOT)/src -I$(PROJECT_ROOT)/include -I$(SDK)

TARGETNAME := liblog# 库文件名称
STATIC := a
SHARED := so

CXXFLAGS := -Wall -O3 $(INCLUDE_PATH) -fPIC -fvisibility=hidden

SRC_DIR := src
SRC_FILES := $(SRC_DIR)/config/log_config.cpp \
             $(SRC_DIR)/entry/log_entry.cpp \
             $(SRC_DIR)/entry/format/log_format.cpp \
             $(SRC_DIR)/file/log_file_manager.cpp \
             $(SRC_DIR)/utils/log_queue.cpp \
             $(SRC_DIR)/utils/time_stamp.cpp \
             $(SRC_DIR)/log.cpp

OBJ_FILES := $(SRC_FILES:.cpp=.o)

.PHONY: all lib clean

all: static shared

static: $(OBJ_FILES)
	$(AR) rcs $(TARGETNAME).$(STATIC) $^  # 使用 ar 命令创建静态库

shared:	$(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -shared -o $(TARGETNAME).$(SHARED) $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) $(TARGETNAME).$(STATIC) $(TARGETNAME).$(SHARED)