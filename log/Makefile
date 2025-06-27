include ../config.mk

TARGETNAME := liblog# 库文件名称
STATIC := a

SRC_DIR := src
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp)

OBJ_FILES := $(SRC_FILES:.cpp=.o)

.PHONY: clean static

all: static

static: $(TARGETNAME).$(STATIC)

# $@只是当前规则的目标文件名
$(TARGETNAME).$(STATIC): $(OBJ_FILES)
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJ_FILES) $(TARGETNAME).$(STATIC)