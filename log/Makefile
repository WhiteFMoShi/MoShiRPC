include ../config.mk

TARGETNAME := liblog# 库文件名称
STATIC := a

SRC_DIR := src
SRC_FILES := $(SRC_DIR)/config/log_config.cpp \
             $(SRC_DIR)/entry/log_entry.cpp \
             $(SRC_DIR)/entry/format/log_format.cpp \
             $(SRC_DIR)/file/log_file_manager.cpp \
             $(SRC_DIR)/utils/log_queue.cpp \
             $(SRC_DIR)/utils/time_stamp.cpp \
             $(SRC_DIR)/log.cpp

OBJ_FILES := $(SRC_FILES:.cpp=.o)

.PHONY: clean static

all: static

static: $(LIBS_PATH)/$(TARGETNAME).$(STATIC)

# $@只是当前规则的目标文件名
$(LIBS_PATH)/$(TARGETNAME).$(STATIC): $(OBJ_FILES)
	$(shell mkdir -p $(LIBS_PATH))
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJ_FILES) $(LIBS_PATH)/$(TARGETNAME).$(STATIC)