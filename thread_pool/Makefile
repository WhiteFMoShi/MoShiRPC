include ../config.mk

TARGETNAME := libthreadpool
STATIC := a

SRC_DIR := src
SRC_FILES := $(SRC_DIR)/thread_pool.cpp
OBJ_FILES := $(SRC_FILES:.cpp=.o)

all: static

static:$(TARGETNAME).$(STATIC)

$(TARGETNAME).$(STATIC): $(OBJ_FILES)
	$(AR) rcs $@ $^

.PHONY: clean
clean: 
	rm -f $(OBJ_FILES) $(TARGETNAME).$(STATIC)